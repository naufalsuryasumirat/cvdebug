#ifndef TB_PARAMS_H
#define TB_PARAMS_H

#include <any>
// #include <memory>
#include <string>
#include <variant>
#include <vector>

namespace tb {

struct p;
typedef struct p p;

struct dt {
    std::variant<int*, float*, double*> v;
    int max,
        min = 0;
    void (*cb)(std::variant<std::any>*) = NULL;
};

struct _pg {
    std::vector<p> children;
    _pg operator ,(p& b);
};

struct p {
    std::string name;
    std::vector<p> children;
    bool has_data = false;
    dt d;

    p();
    p(const char* n);
    p(const char* n, dt d);
    p(const char* n, const struct p& other);
    p(const char* n, const struct _pg& group);

    _pg operator ,(const p& b);
    p& operator =(const p& b);
};

}

#endif // !TB_PARAMS_H
