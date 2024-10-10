#include <iostream>
#include "pinc/pinc.hpp"

using namespace coen;

pinc::coroutine<int> bar()
{
    for (int i = 0; i < 10; i++) {
        co_yield i;
    }
}

pinc::coroutine<int> foo()  
{
    std::cout << "in foo" << std::endl;
    co_return 5;
}

pinc::coroutine<int> baz() {
    co_return co_await bar();
}


pinc::coroutine<int> root() {
    while (true) {
        std::cout << co_await baz() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    return pinc::start(root());
}