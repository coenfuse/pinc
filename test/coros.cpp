// standard imports
#include <chrono>
#include <iostream>

// external imports
#include "../pinc/pinc.hpp"



coen::pinc::coroutine<int> generator() 
{
    for (int i = 0; i < 10; i ++) {
        co_yield i;
    }
}


coen::pinc::coroutine<int> useless_coro()
{
    std::cout << "in useless coroutine" << std::endl;
    co_return 5;
}


coen::pinc::coroutine<int> awaiting_coro()
{
    std::cout << "in useful coroutine" << std::endl;
    int response = co_await useless_coro();
    std::cout << "back in useful coroutine" << std::endl;
    co_return response; 
}


coen::pinc::coroutine<> root()
{
    std::cout << co_await awaiting_coro() << std::endl;
    auto num_gen = generator();
    while (!num_gen.is_done()) {
        std::cout << num_gen.yield_next().value_or(0) << std::endl;
    }
}


int main()
{
    coen::pinc::start(root());
}