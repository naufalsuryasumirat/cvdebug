#include <iostream>
#include <deque>

#include "tb_params.h"

tb::pg tb::pg::operator, (const p& b)
{
    pg group(*this);
    group.children.push_back(b);
    return group;
}

tb::p::p(const char* n)
    : name(n)
{
}

tb::p::p(const char* n, dt d)
    : name(n)
    , d(d)
{
    this->has_data = true;
}

tb::p::p(const char* n, struct p&& other)
    : name(n)
    , children({other})
{
}

tb::p::p(const char* n, struct pg&& group)
    : name(n)
    , children(group.children)
{
}
    
tb::pg tb::p::operator ,(const p& b)
{
    pg group;
    group.children.push_back(*this);
    group.children.push_back(b);
    return group;
}

tb::p& tb::p::operator =(const p& b)
{
    this->name = b.name;
    this->children = b.children;
    this->has_data = b.has_data;
    this->d = b.d;
    return *this;
}

void tbd::bfsp(const tb::p& params)
{
    std::deque<std::pair<int, const tb::p*>> d = {{0,&params}};
    while (d.size()) {
        auto& [depth, param] = d.front();
        std::cout << std::string(depth, ' ') << param->name << std::endl;
        for (const auto& c : param->children)
            d.push_back({depth+1,&c});
        d.pop_front();
    }
}

void tbd::dfsp(const tb::p& params, const uint16_t depth)
{
    std::cout << std::string(depth, ' ') << params.name << std::endl;
    for (size_t i = 0; i < params.children.size(); ++i) {
        dfsp(params.children[i], depth+1);
    }
}

void tbd::dfsp_d(const tb::p& params, const uint16_t depth)
{
    fmt::println("d[{:02}]: {}, child: {}", depth, params.name, params.children.size());
    if (params.has_data) {
        std::visit([](auto&& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, int*>) {
                fmt::println("\t-> int: {}", *v);
            } else if constexpr (std::is_same_v<T, float*>) {
                fmt::println("\t-> float: {:.2f}", *v);
            } else if constexpr (std::is_same_v<T, double*>) {
                fmt::println("\t-> double: {:.3f}", *v);
            } else if constexpr (std::is_same_v<T, bool*>) {
                fmt::println("\t-> bool: {}", *v);
            } else { fmt::println("\t-> unknown"); }
        }, params.d.v);
    }

    for (size_t i = 0; i < params.children.size(); ++i) {
        // dfsp_d(*params.children[i], depth+1);
        dfsp_d(params.children[i], depth+1);
    }
}
