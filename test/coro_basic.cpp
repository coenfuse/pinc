#include <coroutine>
#include <iostream>
#include <memory>

class async;
class async_promise;

class async
{
    public:
        using promise_type = class async_promise;
        using t_handle = std::coroutine_handle<promise_type>;
        friend promise_type;

    public:

        void resume();
        int get_result();

    private:
        explicit async (t_handle* handle);

    private:
    t_handle* muptr_handle;
};

class async_promise
{
    public:

        async get_return_object();

        std::suspend_always initial_suspend();

        std::suspend_always final_suspend() noexcept;

        void unhandled_exception() noexcept;

        void return_value(const int to_return);

        friend class async;

    private:
    std::unique_ptr<async::t_handle> muptr_handle;
    int m_return_value;
};

async::async(async::t_handle* handle) {
    muptr_handle = handle;
}

void async::resume() {
    muptr_handle->resume();
}

int async::get_result() {
    return muptr_handle->promise().m_return_value;
}

async async_promise::get_return_object() {
    muptr_handle = std::make_unique<async::t_handle>(async::t_handle::from_promise(*this));
    return async(muptr_handle.get());
}

std::suspend_always async_promise::initial_suspend() {
    // std::cout << "initial_suspend" << std::endl;
    return  {};
}

std::suspend_always async_promise::final_suspend() noexcept {
    // std::cout << "final_suspend" << std::endl;
    return  {};
}

void async_promise::unhandled_exception() noexcept {
}

void async_promise::return_value(const int to_return) {
    // std::cout << "returning value" << std::endl;
    m_return_value = to_return;
}

async root() {
    // std::cout << "in coro" << std::endl;
    co_return 5;
}

int main() {
    // std::cout << "in main" << std::endl;
    auto coro = root();
    coro.resume();
    std::cout << coro.get_result() << std::endl;
}