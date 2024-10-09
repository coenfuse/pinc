#include <iostream>
#include "pinc/pinc.hpp"


coen::pinc::coroutine<int> bar()
{
    for (int i = 0; i < 10; i++) {
        co_yield i;
    }
}


coen::pinc::coroutine<int> foo()  
{
    std::cout << "in coro" << std::endl;
    co_return 5;
}


int main()
{
    std::cout << "in main" << std::endl;
    auto coroutine = bar();
    auto coro2 = foo();
    coro2.resume();
    std::cout << coro2.yield_next().value() << std::endl;

    while (!coroutine.is_done()) {
        std::cout << "yielded " << coroutine.yield_next().value() << std::endl;
    }

    std::cout << "back in main" << std::endl;
}