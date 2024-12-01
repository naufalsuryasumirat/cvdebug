#ifndef TB_PARAMS_H
#define TB_PARAMS_H

#include <any>
#include <cstdint>
#include <fmt/core.h>
#include <string>
#include <variant>
#include <vector>

namespace tb {

struct p;
typedef struct p p;

struct dt {
    std::variant<int*, float*, double*, bool*> v;
    int max,
        min = 0;
    void (*cb)(std::variant<std::any>*) = NULL;
};

struct pg {
    std::vector<p> children;

    pg operator ,(const p& b);
};

struct p {
    std::string name;
    std::vector<p> children;
    bool has_data = false;
    dt d;

    p(const char* n);
    p(const char* n, dt d);
    p(const char* n, struct p&& other);
    p(const char* n, struct pg&& group);
    
    pg operator ,(const p& b);
    p& operator =(const p& b);
};

}

namespace tbd {

void bfsp(const tb::p& params);
void dfsp(const tb::p& params, const uint16_t depth = 0);
void dfsp_d(const tb::p& params, const uint16_t depth = 0);

}

#endif // !TB_PARAMS_H
