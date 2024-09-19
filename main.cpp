#include <iostream>
#include "pinc/pinc.hpp"

// #define async pinc::Awaitable
// #define await pinc::await
using namespace coen;


pinc::Awaitable<> coro_loop(const size_t length) 
{
    return pinc::Awaitable<>(
        [&](size_t length)
        {
            for (size_t itr = 0; itr <= length; itr++) 
            {
                std::cout << "coro[" << length << "] -> itr " << itr << std::endl;
                pinc::await(pinc::sleep(std::chrono::seconds(1)));
            }
        }
    );
}

void sync_job()
{
    std::cout << "coro[5] -> started on thread" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "coro[5] -> finished on thread" << std::endl;
}

pinc::Awaitable<> root()
{
    return pinc::Awaitable<>(
        [](){
            std::vector<pinc::Awaitable<>> tasks;
            for (int i = 0; i < 3; i++) {
                tasks.push_back(coro_loop(i * 1));
            }
            tasks.push_back(pinc::to_thread(std::function<void()>(&sync_job)));
            pinc::await(pinc::gather(tasks));
        }
    );
}

int main() {
    return pinc::start(root());
}


/*
Expected Output

> coro[1] -> itr 0
> coro[4] -> itr 0
> coro[9] -> itr 0
> coro[5] -> started on thread
> coro[1] -> itr 1
> coro[4] -> itr 1
> coro[9] -> itr 1
> coro[4] -> itr 2
> coro[9] -> itr 2
> coro[4] -> itr 3
> coro[9] -> itr 3
> coro[4] -> itr 4
> coro[9] -> itr 4
> coro[5] -> finished on thread
> coro[9] -> itr 5
> coro[9] -> itr 6
> coro[9] -> itr 7
> coro[9] -> itr 8
> coro[9] -> itr 9

*/