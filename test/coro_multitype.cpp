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
        
        friend class context<T>;
        using promise_type = class context<T>;
        using coro_handle  = std::coroutine_handle<promise_type>;

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
        // todo - destructor

    private:

        std::coroutine_handle<> m_caller_handle;
        coro_handle* mptr_self_handle;
};



template <>
class coen::pinc::async<void>
{
    public:

        friend class context<>;
        using promise_type = class context<>;
        using coro_handle  = std::coroutine_handle<promise_type>;
    
    public:

        bool await_ready() const;
        void await_suspend(std::coroutine_handle<> caller_handle);
        void await_resume() const;
    
        void run() const;
        void cancel() const;
        bool is_done() const;

    private:

        explicit async(coro_handle* ptr_handle);
        // todo - destructor

    private:

        std::coroutine_handle<> m_caller_handle;
        coro_handle* mptr_self_handle;
};




template <typename T>
class coen::pinc::context
{
    public:

        friend class async<T>;
        using coro_handle = typename async<T>::coro_handle;

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



template <>
class coen::pinc::context<void>
{
    public:

        friend class async<void>;
        using coro_handle = typename async<void>::coro_handle;

    public:

        async<void> get_return_object();
        std::suspend_always initial_suspend();
        std::suspend_always final_suspend() noexcept;
        void unhandled_exception() noexcept;
        void return_void() noexcept;

    public:

        // explicit context();
        // ~context();

    private:

        std::unique_ptr<coro_handle> muptr_handle;
};




template <typename T>
coen::pinc::async<T>::async(coro_handle* ptr_handle) {
    mptr_self_handle = ptr_handle;
}


coen::pinc::async<void>::async(coro_handle* ptr_handle) {
    mptr_self_handle = ptr_handle;
}


template <typename T>
bool coen::pinc::async<T>::await_ready() const {
    return this->is_done();
}


bool coen::pinc::async<void>::await_ready() const {
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


void coen::pinc::async<void>::await_suspend(
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


void coen::pinc::async<void>::await_resume() const {
}


template <typename T>
void coen::pinc::async<T>::run() const {
    mptr_self_handle->resume();
}


void coen::pinc::async<void>::run() const {
    mptr_self_handle->resume();
}


template <typename T>
void coen::pinc::async<T>::cancel() const {
    mptr_self_handle->destroy();    // not sure if this should be invoked since next call to resume might cause segmentation fault. But its the scheduler that should ensure it is not called again, so maybe this can be mitigated.
}


void coen::pinc::async<void>::cancel() const {
    mptr_self_handle->destroy();
}


template <typename T>
bool coen::pinc::async<T>::is_done() const {
    return mptr_self_handle->done();
}


bool coen::pinc::async<void>::is_done() const {
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


coen::pinc::async<void> coen::pinc::context<void>::get_return_object()
{
    muptr_handle = std::make_unique<coro_handle>(coro_handle::from_promise(*this));
    return async<void>(muptr_handle.get());
}


template <typename T>
std::suspend_always coen::pinc::context<T>::initial_suspend() {
    return {};
}


std::suspend_always coen::pinc::context<void>::initial_suspend() {
    return {};
}


template <typename T>
std::suspend_always coen::pinc::context<T>::final_suspend() noexcept {
    return {};
}


std::suspend_always coen::pinc::context<void>::final_suspend() noexcept {
    return {};
}


template <typename T>
void coen::pinc::context<T>::unhandled_exception() noexcept {
    std::terminate();       // remove this and handle better
}


void coen::pinc::context<void>::unhandled_exception() noexcept {
    std::terminate();
}


template <typename T>
void coen::pinc::context<T>::return_value(T to_return) noexcept {
    m_return_value = to_return;
}


void coen::pinc::context<void>::return_void() noexcept {
}


template <typename T>
T coen::pinc::context<T>::get_return_value() const {
    return m_return_value;
}




coen::pinc::async<int> node() {
    std::cout << "node() - start" << std::endl;
    co_return 15;
    std::cout << "node() - exit" << std::endl;
}

coen::pinc::async<int> leaf() {
    std::cout << "leaf() - start" << std::endl;
    co_return 5;
    std::cout << "leaf() - exit" << std::endl;
}

coen::pinc::async<int> branch() {
    std::cout << "branch() - start" << std::endl;
    int response = co_await leaf();
    std::cout << "branch() - still" << std::endl;
    co_return response;
    std::cout << "branch() - exit" << std::endl;
}

coen::pinc::async<> root() {
    std::cout << "root() - start" << std::endl;
    int response = co_await branch() + co_await node();
    std::cout << "root() - still" << std::endl;
    std::cout << response << std::endl;
    std::cout << "root() - exit" << std::endl;
}

int main() 
{
    std::cout << "main() - start" << std::endl;
    auto coro = root();
    std::cout << "main() - still" << std::endl;
    coro.run();
    std::cout << "main() - exit" << std::endl;
}
