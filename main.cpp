#include <iostream>
#include "pinc/pinc.hpp"


coen::pinc::coroutine<int> bar()
{
    for (int i = 0; i < 10; i++)
    {
        std::cout << "yielding " << i << std::endl;
        co_yield i;
    }
}

/*
coen::pinc::coroutine<> foo()
{
    std::cout << "in coro" << std::endl;
    co_return;
}
*/

int main()
{
    std::cout << "in main" << std::endl;
    auto coroutine = bar();

    while (1) {
        coroutine.resume();
    }

    coroutine.resume();
    std::cout << "back in main" << std::endl;
}