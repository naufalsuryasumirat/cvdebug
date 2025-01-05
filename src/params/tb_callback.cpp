#include <limits>
#include <stdexcept>
#include <variant>

#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/ranges.h>
#include <fmt/args.h>

#include "tb_callback.h"

namespace {
// NOTE: very flawed, always assumes v and cmp to be the same ptype
void _cb_more_than(arp_var v, arp_var cmp)
{
    std::visit([](auto&& _v, auto&& _cmp) {
        using T = std::decay_t<decltype(_v)>;
        using U = std::decay_t<decltype(_cmp)>;
        if constexpr (not std::is_same_v<T, U>)
            throw std::logic_error("Value and Comparator must be the same pointer type");

        using V = std::remove_pointer_t<T>;
        if constexpr (std::is_same_v<T, int*> || std::is_same_v<T, float*> || std::is_same_v<T, double*>) {
            static V prev = std::numeric_limits<V>::max();
            if (*_v <= *_cmp) {
                *_v = prev;
                if (*_v <= *_cmp) // if still lesseq than
                    *_v = *_cmp + static_cast<V>(1);
            }
            prev = *_v;
        } else throw (std::logic_error(fmt::format("Arithmetic pointer type: {} not supported", typeid(T).name())));
    }, v, cmp);
}

void _cb_less_than(arp_var v, arp_var cmp)
{
    std::visit([](auto&& _v, auto&& _cmp) {
        using T = std::decay_t<decltype(_v)>;
        using U = std::decay_t<decltype(_cmp)>;
        if constexpr (not std::is_same_v<T, U>)
            throw std::logic_error("Value and Comparator must be the same pointer type");

        using V = std::remove_pointer_t<T>;
        if constexpr (std::is_same_v<T, int*> || std::is_same_v<T, float*> || std::is_same_v<T, double*>) {
            static V prev = std::numeric_limits<V>::max();
            if (*_v >= *_cmp) {
                *_v = prev;
                if (*_v >= *_cmp) // if still more than
                    *_v = *_cmp - static_cast<V>(1);
            }
            prev = *_v;
        } else throw (std::logic_error(fmt::format("Arithmetic pointer type: {} not supported", typeid(T).name())));
    }, v, cmp);
}

}

// FIXME: static here might not be wise
void tb::cb_odd(arp_var v)
{
    std::visit([](auto&& _v) {
        using T = std::decay_t<decltype(_v)>;
        if constexpr (std::is_same_v<T, int*>) {
            static int prev = std::numeric_limits<int>::min();
            if (*_v % 2 == 0) {
                const bool increasing = *_v >= prev;
                if (increasing) (*_v)++;
                else (*_v)--;
            }
            prev = *_v;
        } else throw std::logic_error("Not implemented");
    }, v);
}

void tb::cb_even(arp_var v)
{
    std::visit([](auto&& _v) {
        using T = std::decay_t<decltype(_v)>;
        if constexpr (std::is_same_v<T, int*>) {
            static int prev = std::numeric_limits<int>::min();
            if (*_v % 2 == 1) {
                const bool increasing = *_v >= prev;
                if (increasing) (*_v)++;
                else (*_v)--;
            }
            prev = *_v;
        } else throw std::logic_error("Not implemented");
    }, v);
}

std::function<void(arp_var)> tb::cb_more_than(arp_var cmp)
{
    using namespace std::placeholders;
    return std::bind(::_cb_more_than, _1, cmp);
}

std::function<void(arp_var)> tb::cb_less_than(arp_var cmp)
{
    using namespace std::placeholders;
    return std::bind(::_cb_less_than, _1, cmp);
}
