#pragma once

#if !defined(FLOW_OPERATORS_PROJECT_HPP)
#define FLOW_OPERATORS_PROJECT_HPP

#include <rxcpp/rx-includes.hpp>
#include <flow-operators.hpp>

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct project_invalid_arguments {};

template<class... AN>
struct project_invalid : public rxo::operator_base<project_invalid_arguments<AN...>> {
    using type = observable<project_invalid_arguments<AN...>, project_invalid<AN...>>;
};

template<class... AN>
using project_invalid_t = typename project_invalid<AN...>::type;

template<class T, class Selector>
struct project {
    
    typedef rxu::decay_t<T> source_value_type;
    typedef rxu::decay_t<Selector> select_type;
    typedef decltype((*(select_type*)nullptr)(*(source_value_type*)nullptr)) value_type;
    
    select_type selector;
    
    project(select_type s) : selector(std::move(s))
    {
    }
    
    template<class Subscriber>
    struct project_observer 
    {
        typedef project_observer<Subscriber> this_type;
        typedef decltype((*(select_type*)nullptr)(*(source_value_type*)nullptr)) value_type;
        typedef rxu::decay_t<Subscriber> dest_type;

        typedef observer<source_value_type, this_type> observer_type;
        
        dest_type dest;
        mutable select_type selector;
        
        project_observer(dest_type d, select_type s) : 
            dest(std::move(d)), 
            selector(std::move(s))
        {
        }

        template<class Value> 
        void on_next(Value&& v) const 
        {
            auto selected = on_exception(
                [&]() {
                    return this->selector(std::forward<Value>(v)); },
                dest);
            
            if (selected.empty()) {
                return;
            }
            dest.on_next(std::move(selected.get()));            
        }
        
        void on_error(std::exception_ptr e) const {
            dest.on_error(e);
        }

        void on_completed() const {
            dest.on_completed();
        }

        static subscriber<source_value_type, observer_type> make(dest_type d, select_type s) {
            auto cs = d.get_subscription();
            return make_subscriber<source_value_type>(std::move(cs), observer_type(this_type(std::move(d), std::move(s))));
        }        
    };

    template<class Subscriber>
    auto operator()(Subscriber dest) const 
        -> decltype(project_observer<Subscriber>::make(std::move(dest), selector)) {
        return project_observer<Subscriber>::make(std::move(dest), selector);
    }
};
}

template<class... AN>
auto project(AN&&... an)
    -> operator_factory<project_tag, AN...>
{
    return operator_factory<project_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}
}

template<>
struct member_overload<project_tag>
{
    template<class Observable, class Selector,
             class Enabled = rxu::enable_if_all_true_type_t<
                 is_observable<Observable>>,
             class ResolvedSelector = rxu::decay_t<Selector>,
             class SourceValue = rxu::value_type_t<Observable>,
             class Project = rxo::detail::project<SourceValue, ResolvedSelector>,
             class Value = rxu::value_type_t<Project>>
    static auto member(Observable&& o, Selector&& s) 
        -> decltype(o.template lift<Value>(Project(std::forward<Selector>(s)))) 
    {
        return o.template lift<Value>(Project(std::forward<Selector>(s)));
    }

    template<class... AN>
    static operators::detail::project_invalid_t<AN...> member(const AN...) {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "project takes Selector");
    }
};

}

#endif
