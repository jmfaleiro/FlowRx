#pragma once

#if !defined(FLOW_OPERATORS_SELECT_HPP)
#define FLOW_OPERATORS_SELECT_HPP

#include <rxcpp/rx-includes.hpp>
#include <flow-operators.hpp>

namespace rxcpp {

namespace operators {

namespace detail {

template<class... AN>
struct select_invalid_arguments{};

template<class... AN>
struct select_invalid : public rxo::operator_base<select_invalid_arguments<AN...>> {
    using type = observable<select_invalid_arguments<AN...>, select_invalid<AN...>>;
};

template<class... AN>
using select_invalid_t = typename project_invalid<AN...>::type;

template<class T, class Predicate>
struct select {

    typedef rxu::decay_t<T> source_value_type;
    typedef rxu::decay_t<Predicate> predicate_type;
    typedef source_value_type value_type;
    
    predicate_type predicate;
    
    select(predicate_type p) : predicate(std::move(p))
    {
    }

    template<class Subscriber>
    struct select_observer
    {
        typedef select_observer<Subscriber> this_type;
        typedef source_value_type value_type;
        typedef rxu::decay_t<Subscriber> dest_type;
        
        typedef observer<source_value_type, this_type> observer_type;
        
        dest_type dest;
        predicate_type predicate;
        
        select_observer(dest_type d, predicate_type p) :
            dest(d),
            predicate(p) 
        {
        }
        
        template<class Value>
        void on_next(Value&& v) const 
        {
            auto selected = on_exception(
                [&] () {
                    return this->predicate(v);
                }, 
                dest);
            if (selected.empty()) {
                return;
            }             
            if (selected.get()) {
                dest.on_next(v);
            }
        }
        
        void on_error(std::exception_ptr e) const 
        {
            dest.on_error(e);
        }

        void on_completed() const
        {
            dest.on_completed();
        }
        
        static subscriber<value_type, observer_type> make(dest_type d, predicate_type p)
        {
            return make_subscriber<value_type>(d, this_type(d, std::move(p)));
        }
    };
    
    template<class Subscriber>
    auto operator()(Subscriber dest) const 
        -> decltype(select_observer<Subscriber>::make(std::move(dest), predicate)) 
    {
        return select_observer<Subscriber>::make(std::move(dest), predicate);
    }
};    

}

template<class... AN>
auto select(AN&&... an)
    -> operator_factory<select_tag, AN...>
{
    return operator_factory<select_tag, AN...>(std::make_tuple(std::forward<AN>(an)...));
}

}

template<>
struct member_overload<select_tag>
{
    template<class Observable, class Predicate,
             class SourceValue = rxu::value_type_t<Observable>,
             class Select = rxo::detail::select<SourceValue, rxu::decay_t<Predicate>>>
    static auto member(Observable&& o, Predicate&& p) 
        -> decltype(o.template lift<SourceValue>(Select(std::forward<Predicate>(p))))
    {
        return o.template lift<SourceValue>(Select(std::forward<Predicate>(p)));
    }

    template<class... AN>
    static operators::detail::select_invalid_t<AN...> member(const AN&...)
    {
        std::terminate();
        return {};
        static_assert(sizeof...(AN) == 10000, "select takes Predicate");
    }
};

}

#endif
