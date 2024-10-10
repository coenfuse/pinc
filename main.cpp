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
    co_return co_await foo();
}


pinc::coroutine<int> root() {
    std::cout << co_await baz() << std::endl;
}

int main() {
    return pinc::start(root());
}

// std::cout << "in main" << std::endl;
// int response = co_await baz();
// std::cout << "again back in main" << std::endl;
// std::cout << response << std::endl;
// std::cout << baz().get_result().value() << std::endl;

/*
std::cout << "in main" << std::endl;
auto coroutine = bar();
auto coro2 = foo();
coro2.resume();
std::cout << coro2.yield_next().value() << std::endl;
while (!coroutine.is_done()) {
    std::cout << "yielded " << coroutine.yield_next().value() << std::endl;
}
std::cout << "back in main" << std::endl;
*/