#include <chrono>
#include <iostream>
#include "pinc/pinc.hpp"

using namespace coen;

pinc::Awaitable<> coro_loop(const size_t length) {
    for (size_t itr = 0; itr <= length; itr++) {
        std::cout << "coro[" << length << "] -> itr " << itr << std::endl;
        pinc::await(pinc::sleep(std::chrono::seconds(1)));
    }
}

pinc::Awaitable<> root()
{
    std::vector<pinc::Awaitable<>> tasks;
    for (int i = 0; i < 3; i++) {
        tasks.push_back(coro_loop(i * i));
    }
    pinc::await(pinc::gather(tasks));
}

int main() {
    return pinc::start(root());
}


/*
Expected Output

> coro[1] -> itr 0
> coro[4] -> itr 0
> coro[9] -> itr 0
> coro[1] -> itr 1
> coro[4] -> itr 1
> coro[9] -> itr 1
> coro[4] -> itr 2
> coro[9] -> itr 2
> coro[4] -> itr 3
> coro[9] -> itr 3
> coro[4] -> itr 4
> coro[9] -> itr 4
> coro[9] -> itr 5
> coro[9] -> itr 6
> coro[9] -> itr 7
> coro[9] -> itr 8
> coro[9] -> itr 9

*/