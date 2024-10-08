// #include <coroutine>
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

/*
namespace pinc
{
    // class awaiter;
    class coroutine;

    void start(coroutine coro);   
}

/*
class pinc::awaiter
{
    bool await_ready() noexcept;
    void await_suspend(coroutine::handle_type handle) noexcept;
    void await_resume() noexcept;
};

class pinc::coroutine
{
    public:
    
    class promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    public:
    bool is_resumable();
    uint resume();
    bool is_done();
    uint destroy();

    private:
    explicit coroutine(handle_type handle);

    private:
    handle_type m_handle;
};

class pinc::coroutine::promise_type
{
    public:
    pinc::coroutine get_return_object();
    std::suspend_always initial_suspend();
    void return_void();
    void unhandled_exception() noexcept;
    std::suspend_always final_suspend() noexcept;
    std::suspend_always await_transform(pinc::coroutine coro);
    std::suspend_always yield_value(int value);

    private:
    pinc::coroutine::handle_type m_handle;
};

// -----------------------------------------------------------------------------
// Method Implementations
// -----------------------------------------------------------------------------

pinc::coroutine::coroutine(pinc::coroutine::handle_type handle) 
{
    m_handle = handle;
}


bool pinc::coroutine::is_resumable()
{
    return true;
}


uint pinc::coroutine::resume()
{
    if (is_resumable()) 
    {
        m_handle.resume();
        return 0;
    }
    return 1;
}


bool pinc::coroutine::is_done()
{
    return m_handle.done();
}


uint pinc::coroutine::destroy()
{
    try
    {
        m_handle.destroy();
        return 0;
    }
    catch (...)
    {
        return 1;
    }
}


pinc::coroutine pinc::coroutine::promise_type::get_return_object()
{
    m_handle = pinc::coroutine::handle_type::from_promise(*this);
    return pinc::coroutine(m_handle);
}


std::suspend_always pinc::coroutine::promise_type::initial_suspend()
{
    return {};
}


void pinc::coroutine::promise_type::return_void()
{
}


void pinc::coroutine::promise_type::unhandled_exception() noexcept
{
}


std::suspend_always pinc::coroutine::promise_type::final_suspend() noexcept
{
    return {};
}


std::suspend_always pinc::coroutine::promise_type::await_transform(
    pinc::coroutine coro
)
{
    return {};
}


std::suspend_always pinc::coroutine::promise_type::yield_value(
    int value
)
{
    return {};
}

// -----------------------------------------------------------------------------
// DRIVER
// -----------------------------------------------------------------------------

pinc::coroutine bar()
{
    for (int i = 0; i < 10; i++)
    {
        std::cout << "yielding " << i << std::endl;
        co_yield i;
    }
}


pinc::coroutine foo()
{
    std::cout << "in coroutine" << std::endl;
    co_await foo();
}


int main() 
{
    std::cout << "in main" << std::endl;
    auto coroutine = bar();
    
    while (1) {
        coroutine.resume();
    }

    std::cout << "back in main" << std::endl;
}
*/