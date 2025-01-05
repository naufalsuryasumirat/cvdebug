#include <functional>
#include <variant>

typedef std::variant<int*, float*, double*, bool*> arp_var; // arithmetic pointer variant

namespace tb {

void cb_odd(arp_var v);
void cb_even(arp_var v);
std::function<void(arp_var)> cb_more_than(arp_var cmp);
std::function<void(arp_var)> cb_less_than(arp_var cmp);

}
