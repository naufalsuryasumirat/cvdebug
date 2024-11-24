#include <opencv.hpp>

#include "src/trackbar.h"

void func()
{
    std::shared_ptr<bool> debugging = tb::init(true);

    // TODO: dangerous rn, can't quit
    do {
        tb::update();

        fmt::print("count: {}, value: {}\n", debugging.use_count(), *debugging);

        tb::view();
    } while (*debugging);
}

int main()
{
    func();

    return 0;
}

