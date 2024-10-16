// standard imports
#include <chrono>
#include <coroutine>
#include <iostream>
#include <memory>

// external imports
// #include "../pinc/pinc.hpp"



/*
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


coen::pinc::coroutine<> roots()
{
    std::cout << co_await awaiting_coro() << std::endl;
    auto num_gen = generator();
    while (!num_gen.is_done()) {
        std::cout << num_gen.yield_next().value_or(0) << std::endl;
    }
}
*/

/**

coen::pinc::coroutine<int> coro2() {
    co_return 1;
}

coen::pinc::coroutine<int> coro1() {
    co_return co_await coro2();
}

coen::pinc::coroutine<> root() {
    int response = co_await coro1();
    std::cout << "cout " << response << std::endl;
}


int main()
{
    coen::pinc::start(root());
}
*/

template <typename T>
class awaiter;

template <typename T = void>
class async;

template <typename T = void>
class promise_type;


template <typename T>
class async
{
    public:

    // remove this and put it inside destructor?
    void destroy() {
        // maybe consider this to be done by internal promise_type
    }
    
    bool is_done() {
        return m_handle->done();
    }

    void resume() {
        std::cout << "resuming coro" << std::endl;
        return m_handle->resume();
    }

    using promise_type = class promise_type<T>;                 // because compiler expects a type 'promise_type' inside coroutine shell
    using t_handle = std::coroutine_handle<promise_type>;       // create a handler from it pure and simple
    friend promise_type;                                        // give this promise_type access to private methods of async

    private:
    async(t_handle* handle) {
        m_handle = handle;
    }

    t_handle* m_handle;
};


template <typename T>
class promise_type
{
    public:
    
    async<T> get_return_object() 
    {
        std::cout << "creating an awaitable for called from its promise" << std::endl;
        mptr_handle = std::make_unique<t_handle>(t_handle::from_promise(*this));
        return async<T>(mptr_handle.get());
    }
    
    std::suspend_always initial_suspend() 
    {
        std::cout << "entering coro so called promise.initial_suspend()" << std::endl;
        return {};
    }

    std::suspend_always final_suspend() noexcept 
    {
        std::cout << "existing coro (probably after co_return) so called promise.final_suspend()" << std::endl;
        return {};
    }
    
    void return_void() noexcept {
        std::cout << "detected co_return in coro so called promise.return_void()" << std::endl;
    }

    void unhandled_exception() noexcept {
    }
    
    // disabled since we are using get_return_object() as proxy ctor
    // promise_type() {
    //     std::cout << "initializing handle for coro from its promise" << std::endl;
    // }

    private:
    using t_handle = typename async<T>::t_handle;
    std::unique_ptr<t_handle> mptr_handle;
};


async<> foo() {
    std::cout << "in coro" << std::endl;
    co_return;
    std::cout << "exiting main" << std::endl;       // never executes
}

int main() {
    std::cout << "in main" << std::endl;
    auto coro = foo();
    std::cout << "still in main" << std::endl;
    coro.resume();
}