#include "tb_params.h"

constexpr char* NULL_NAME = const_cast<char*>("trackbar[placeholder]");

tb::p::p()
    : name(NULL_NAME)
{
}

tb::pg tb::pg::operator ,(p&& b)
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

tb::p::p(tb::pg&& group)
    : name(NULL_NAME)
    , children(group.children)
{
}

tb::p::p(std::initializer_list<p> ps)
    : name(NULL_NAME)
    , children(ps)
{
}
    
tb::pg tb::p::operator ,(p&& b)
{
    pg group;
    group.children.push_back(*this);
    group.children.push_back(b);
    return group;
}

// tb::p& tb::p::operator =(const p& b)
// {
//     this->name = b.name;
//     this->children = b.children;
//     this->has_data = b.has_data;
//     this->d = b.d;
//     return *this;
// }
