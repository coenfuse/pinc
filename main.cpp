#include <iostream>
#include "pinc/pinc.hpp"


coen::pinc::coroutine<int> bar()
{
    for (int i = 0; i < 10; i++) {
        co_yield i;
    }
}


coen::pinc::coroutine<> foo()  
{
    std::cout << "in coro" << std::endl;
    co_return;
}


int main()
{
    std::cout << "in main" << std::endl;
    auto coroutine = bar();
    auto coro2 = foo();
    coro2.resume();

    while (!coroutine.is_done()) {
        std::cout << "yielded " << coroutine.yield_next() << std::endl;
    }

    std::cout << "back in main" << std::endl;
}