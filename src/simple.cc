#include "rxcpp/rx.hpp"

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

void join_test()
{       
    auto stream1 = rxcpp::observable<>::interval(std::chrono::milliseconds(1000)).map([] (int v) { return make_tuple(0, v); });
    auto stream2 = rxcpp::observable<>::interval(std::chrono::milliseconds(1000)).map([] (int v) { return make_tuple(1, v); });
    
    auto join_stream = stream1.merge(stream2);
    
    join_stream |
        flat_map([] (auto t) -> rxcpp::observable<tuple<int,int>> {
                if (get<0>(t) == 0) {
                    stream1_map[get<1>(t)].push_back(get<1>(t));
                    return rxcpp::observable<>::iterate(stream2_map[get<1>(t)]) | 
                        rxcpp::operators::map([t] (int s2_val) { return make_tuple(get<1>(t), s2_val); });
                } else {
                    stream2_map[get<1>(t)].push_back(get<1>(t));
                    return rxcpp::observable<>::iterate(stream1_map[get<1>(t)]) | 
                        rxcpp::operators::map([t] (int s1_val) { return make_tuple(s1_val, get<1>(t)); });
                }}) |
        subscribe<tuple<int,int>>([] (auto t) { std::cout << "<" << get<0>(t) << "," << get<1>(t) << ">\n"; });
}

int main()
{
    join_test();
    return 0;
}
