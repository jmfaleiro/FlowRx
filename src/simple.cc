#include "rxcpp/rx.hpp"
#include <join.h>
#include <operators/flow-project.hpp>
#include <operators/flow-select.hpp>

namespace Rx {
using namespace rxcpp;
using namespace rxcpp::sources;
using namespace rxcpp::operators;
using namespace rxcpp::util;
}
using namespace Rx;

#include <regex>
#include <random>
#include <unordered_map>
using namespace std;
using namespace std::chrono;


unordered_map<int, vector<int>> stream1_map;
unordered_map<int, vector<int>> stream2_map;

tuple<int, int> prev_t1;
tuple<int, int, int> prev_t2;
uint64_t v1 = 0;
uint64_t v2 = 0;
vector<tuple<int, int>> tuples1;
vector<tuple<int, int, int>> tuples2;

void join_test()
{       
    auto s1_vals = {4, 4, 1, 4, 5};
    auto stream1 = rxcpp::observable<>::iterate(s1_vals).map([] (int v) { return make_tuple(0, v); });
    auto stream2 = rxcpp::observable<>::interval(std::chrono::milliseconds(1000)).map([] (int v) { return make_tuple(1, (v % 5)); });
    
    fl::Selector<int, tuple<int, int>> s = [] (auto t) { return get<1>(t); };
    fl::Combiner<tuple<int, int, int, int>, tuple<int, int>, tuple<int, int>> comb = [] (auto t1, auto t2) { return tuple_cat(t1, t2); };
    
    //    auto joined_stream = stream1 fl::| fl_join<tuple<int,int>,tuple<int,int>,int,tuple<int,int,int,int>>(stream2, s, s, comb);
    auto joined_stream = fl::join<tuple<int,int>,tuple<int,int>,int,tuple<int,int,int,int>>()(stream1, stream2, s, s, comb);
    joined_stream | subscribe<tuple<int,int, int, int>>([] (auto t) { std::cout << "<" << get<1>(t) << "," << get<3>(t) << ">\n"; });
}

void combine_test()
{
    auto stream1 = rxcpp::observable<>::interval(std::chrono::milliseconds(1000)).map([] (int v) { return make_tuple(0, v); });
    auto stream2 = rxcpp::observable<>::interval(std::chrono::milliseconds(1000)).map([] (int v) { return make_tuple(1, v, v); });
    
    auto join_stream1 = stream1 | rxcpp::operators::map([] (auto t) { tuples1.push_back(t); return make_tuple(t, tuples1); });
    auto join_stream2 = stream2 | rxcpp::operators::map([] (auto t) { tuples2.push_back(t); return make_tuple(t, tuples2); });

    join_stream1 | 
        combine_latest([] (auto t1, auto t2) -> rxcpp::observable<tuple<int, int>> {
                if (get<1>(t1).size() == v1) {
                    v2 += 1;
                    return rxcpp::observable<>::iterate(get<1>(t1)) |
                        filter([t2] (auto t) { return get<1>(t) == get<1>(get<0>(t2)); }) |
                        rxcpp::operators::map([t2] (auto t) { return make_tuple(get<1>(t), get<1>(get<0>(t2))); });
                } else {
                    v1 += 1;
                    return rxcpp::observable<>::iterate(get<1>(t2)) |
                        filter([t1] (auto t) {return get<1>(t) == get<1>(get<0>(t1)); }) |
                        rxcpp::operators::map([t1] (auto t) { return make_tuple(get<1>(get<0>(t1)), get<1>(t)); });
                }
            }, 
            join_stream2) |
        flat_map([] (auto t) { return t; }) |
    subscribe<tuple<int,int>>([] (auto t) { std::cout << "<" << get<0>(t) << "," << get<1>(t) << ">\n"; });    
}

void project_test()
{
    auto stream = rxcpp::observable<>::interval(std::chrono::milliseconds(1000)).map([] (int v) { return make_tuple(1, v, v); });
    stream | 
    project([] (auto t) {
            return t;
    }) |
    subscribe<tuple<int,int,int>>([] (auto t) {std::cout << "<" << get<0>(t) << "," << get<1>(t) << ">\n"; });
}

void select_test()
{
    auto stream = rxcpp::observable<>::interval(std::chrono::milliseconds(1000)).map([] (int v) { return make_tuple(1, v, v); });
    stream | 
    select([] (auto t) {
            if ((get<1>(t) % 2) == 0)
                return true;
            else
                return false;
    }) |
    subscribe<tuple<int,int,int>>([] (auto t) {std::cout << "<" << get<0>(t) << "," << get<1>(t) << ">\n"; });
}

int main()
{
    //    project_test();
    select_test();
    //    join_test();
    //    combine_test();
    return 0;
}
