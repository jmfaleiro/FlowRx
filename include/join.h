#pragma once

#include <rxcpp/rx.hpp>
#include <tuple>

namespace Rx {
using namespace rxcpp;
using namespace rxcpp::sources;
using namespace rxcpp::operators;
using namespace rxcpp::util;
}
using namespace Rx;

using namespace std;

template<typename X, typename Xs>
    using Selector=function<X(Xs)>;
                                 
template<typename C, typename A, typename B>
    using Combiner=function<C(A, B)>;


template<typename U, typename V, typename T, typename O>
class join
{
 public:

    auto operator()(rxcpp::observable<U> stream1, rxcpp::observable<V> stream2,
                    Selector<T, U> sel1, Selector<T, V> sel2, Combiner<O, U, V> comb) {

        auto join_stream1 = stream1 | 
            rxcpp::operators::map([this] (auto t) { _s1_tuples.push_back(t); 
                                                    return make_tuple(t, _s1_tuples);
                });
    
        auto join_stream2 = stream2 | 
            rxcpp::operators::map([this] (auto t) { _s2_tuples.push_back(t);
                                                    return make_tuple(t, _s2_tuples);
                });
    
        return join_stream1 | 
            combine_latest([sel1, sel2, comb, this] (auto t1, auto t2) -> rxcpp::observable<O> {
                    if (get<1>(t1).size() == _s1_index) {
                        _s2_index += 1;
                        return rxcpp::observable<>::iterate(get<1>(t1)) |
                            filter([sel1, sel2, comb, t2] (auto t) { return sel1(t) == sel2(get<0>(t2)); }) |
                            rxcpp::operators::map([t2, comb] (auto t) { return comb(t, get<0>(t2)); }); 
                    } else {
                        _s1_index += 1;
                        return rxcpp::observable<>::iterate(get<1>(t2)) |
                            filter([sel1, sel2, comb, t1] (auto t) { return sel1(get<0>(t1)) == sel2(t); }) |
                            rxcpp::operators::map([t1, comb] (auto t) { return comb(get<0>(t1), t); });
                    }
                },
                join_stream2) |
            flat_map([] (auto t) { return t; });
    };

 private:
    vector<U> 		_s1_tuples;
    vector<V> 		_s2_tuples;    
    uint64_t 		_s1_index = 0;
    uint64_t 		_s2_index = 0;
};
