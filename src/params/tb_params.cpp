#include "tb_params.h"

tb::_pg tb::_pg::operator, (p& b)
{
    _pg group(*this);
    group.children.push_back(b);
    return group;
}

tb::p::p()
    : name("placeholder")
{
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

tb::p::p(const char* n, const struct p& other)
    : name(n)
    , children({other})
{
}

tb::p::p(const char* n, const struct _pg& group)
    : name(n)
    , children(group.children)
{
}
