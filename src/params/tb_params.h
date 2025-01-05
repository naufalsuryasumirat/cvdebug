#ifndef TB_PARAMS_H
#define TB_PARAMS_H

#include <fmt/core.h>
#include <functional>
#include <string>
#include <variant>
#include <vector>

namespace tb {

struct p;
typedef struct p p;

// TODO: enforce max, min, and v to be the same type, but ignore for boolean ofc
struct dt {
    std::variant<int*, float*, double*, bool*> v;
    int max,
        min = 0;
    std::function<void(std::variant<int*, float*, double*, bool*>)> cb = nullptr;
};

struct pg {
    std::vector<p> children;

    pg operator ,(p&& b);
};

struct p {
    std::string name;
    std::vector<p> children;
    bool has_data = false;
    dt d;

    p();
    p(const char* n);
    p(const char* n, dt d);
    p(const char* n, struct p&& other);
    p(const char* n, struct pg&& group);
    p(pg&& group);
    p(std::initializer_list<p> ps);

    pg operator ,(p&& b);
    // p& operator =(const p& b); // NOTE: should delete?
};

}

#endif // !TB_PARAMS_H
