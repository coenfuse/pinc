#include <coroutine>
#include <iostream>
#include <memory>

template <typename T>
class async;

template <typename T>
class promise_type;

template <typename T>
class async
{
    public: // types & declarations
    
    using promise_type  = class promise_type<T>;
    using T_coro_handle = std::coroutine_handle<promise_type>;
    friend promise_type;

    public: // methods

    private: // methods

    async(T_coro_handle* handle) {
        muptr_handle = handle;
    }

    private: // members

    std::unique_ptr<T_coro_handle> muptr_handle;
};


template <typename T>
class promise_type
{
    public: // types & declarations
    
    using T_coro_handle = typename async<T>::T_coro_handle;
    friend class async<T>;

    public: // methods

    async<T> get_return_object() {
        muptr_handle = std::make_unique<T_coro_handle>(T_coro_handle::from_promise(*this));
        return async<T>(muptr_handle.get());
    }
    
    
    std::suspend_always initial_suspend() {
        return {};
    }
    
    std::suspend_always final_suspend() noexcept {
        return {};
    }
    
    void unhandled_exception() noexcept {
    }
    
    void return_value(T to_return) {
        m_return_value = to_return;
    }

    private: // members

    std::unique_ptr<T_coro_handle> muptr_handle;
    T m_return_value;
};

async<int> root() {
    co_return 5;
}

int main() 
{
    auto coro = root();
    // coro.resume();
}