#include <coroutine>
#include <iostream>
#include <memory>


namespace coen 
{
    namespace pinc
    {
        // -- docs --
        template <typename T = void>
        class async;

        // -- docs --
        template <typename T = void>
        class context;

        // -- docs --
        template <typename T = void>
        class generator;

        // -- docs --
        template <typename T = void>
        class scheduler;

    } // namespace pinc

} // namespace coen

template <typename T>
class coen::pinc::async
{
    public:
        
        using promise_type = class context<T>;
        using coro_handle  = std::coroutine_handle<promise_type>;
        friend promise_type;

    public:

        bool await_ready() const;
        void await_suspend(std::coroutine_handle<> caller_handle);
        T await_resume() const;

        void run() const;       // scheduled to be privatised and only exposed to scheduler
        void cancel() const;    // ..
        bool is_done() const;   // ..

        T get_result() const;   // temporary, remove

    private:

        explicit async(coro_handle* ptr_handle);
        // destructor

    private:

        std::coroutine_handle<> m_caller_handle;
        coro_handle* mptr_self_handle;
};


template <typename T>
class coen::pinc::context
{
    public:

        using coro_handle = typename async<T>::coro_handle;
        friend class async<T>;

    public:

        async<T> get_return_object();
        std::suspend_always initial_suspend();
        std::suspend_always final_suspend() noexcept;
        void unhandled_exception() noexcept;
        void return_value(T to_return) noexcept;

        T get_return_value() const;

    public:

        // explicit context();
        // ~context();

    private:

        std::unique_ptr<coro_handle> muptr_handle;
        T m_return_value;
};


template <typename T>
coen::pinc::async<T>::async(coro_handle* ptr_handle) {
    mptr_self_handle = ptr_handle;
}


template <typename T>
bool coen::pinc::async<T>::await_ready() const {
    return this->is_done();
}


template <typename T>
void coen::pinc::async<T>::await_suspend(
    std::coroutine_handle<> caller_handle
)
{
    m_caller_handle = caller_handle;        // this may break when I disable copy ctor for async
    this->run();
    if (mptr_self_handle->done()) {         // may need to add a special case for awaiting on generators
        m_caller_handle.resume();
    }
}


template <typename T>
T coen::pinc::async<T>::await_resume() const {
    return mptr_self_handle->promise().get_return_value();
}


template <typename T>
void coen::pinc::async<T>::run() const {
    mptr_self_handle->resume();
}


template <typename T>
void coen::pinc::async<T>::cancel() const {
    mptr_self_handle->destroy();    // not sure if this should be invoked since next call to resume might cause segmentation fault. But its the scheduler that should ensure it is not called again, so maybe this can be mitigated.
}


template <typename T>
bool coen::pinc::async<T>::is_done() const {
    return mptr_self_handle->done();
}


template <typename T>
T coen::pinc::async<T>::get_result() const {
    return this->await_resume();
}


template <typename T>
coen::pinc::async<T> coen::pinc::context<T>::get_return_object()
{
    muptr_handle = std::make_unique<coro_handle>(coro_handle::from_promise(*this));
    return async<T>(muptr_handle.get());
}


template <typename T>
std::suspend_always coen::pinc::context<T>::initial_suspend() {
    return {};
}


template <typename T>
std::suspend_always coen::pinc::context<T>::final_suspend() noexcept {
    return {};
}


template <typename T>
void coen::pinc::context<T>::unhandled_exception() noexcept {
    std::terminate();       // remove this and handle better
}


template <typename T>
void coen::pinc::context<T>::return_value(T to_return) noexcept {
    m_return_value = to_return;
}


template <typename T>
T coen::pinc::context<T>::get_return_value() const {
    return m_return_value;
}




coen::pinc::async<int> leaf() {
    co_return 5;
}

coen::pinc::async<int> branch() {
    int response = co_await leaf();
    co_return response;
}

int main() 
{
    auto coro = branch();
    coro.run();
    std::cout << coro.get_result() << std::endl;
}
