#include <coroutine>
#include <iostream>
#include <memory>
#include <string>


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

        async(const async& coroutine) = delete;
        // async(const async&& coroutine);     // TODO 
        ~async() noexcept;

    public:

        bool await_ready() const noexcept;
        void await_suspend(std::coroutine_handle<> caller_handle);
        T await_resume() const;

        void run() const;       // scheduled to be privatised and only exposed to scheduler
        void cancel() const;    // ..
        bool is_done() const;   // ..

        T get_result() const;   // temporary, remove

    private:

        explicit async(coro_handle* ptr_handle);

    private:

        std::coroutine_handle<> m_caller_handle;
        coro_handle* mptr_self_handle = nullptr;
};



template <>
class coen::pinc::async<void>
{
    public:

        friend class context<>;
        using promise_type = class context<>;
        using coro_handle  = std::coroutine_handle<promise_type>;
    
        async(const async& coroutine) = delete;
        // async(const async&& coroutine);     // TODO 
        ~async() noexcept;

    public:

        bool await_ready() const noexcept;
        void await_suspend(std::coroutine_handle<> caller_handle);
        void await_resume() const;
    
        void run() const;
        void cancel() const;
        bool is_done() const;


    private:

        explicit async(coro_handle* ptr_handle);

    private:

        std::coroutine_handle<> m_caller_handle;
        coro_handle* mptr_self_handle = nullptr;
};




template <typename T>
class coen::pinc::context
{
    public:

        friend class async<T>;
        using coro_handle = typename async<T>::coro_handle;

        // explicit context();
        // explicit context(const context& context) = delete;
        // explicit context(const context&& context) = delete;      // consider if needs to be done
        // ~context();

    public:

        async<T> get_return_object();
        std::suspend_always initial_suspend();
        std::suspend_always final_suspend() noexcept;
        void unhandled_exception() noexcept;
        void return_value(const T& to_return) noexcept;
        void return_value(T&& to_return) noexcept;

        T get_return_value() const;

    private:

        void cancel();

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

        // explicit context();
        // explicit context(const context& context) = delete;
        // explicit context(const context&& context) = delete;      // consider if needs to be done
        // ~context();

    public:

        async<void> get_return_object();
        std::suspend_always initial_suspend();
        std::suspend_always final_suspend() noexcept;
        void unhandled_exception() noexcept;
        void return_void() noexcept;

    private:

        void cancel();

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
coen::pinc::async<T>::~async() noexcept {
    mptr_self_handle = nullptr;
}


coen::pinc::async<void>::~async() noexcept {
    mptr_self_handle = nullptr;
}


template <typename T>
bool coen::pinc::async<T>::await_ready() const noexcept {
    return this->is_done();
}


bool coen::pinc::async<void>::await_ready() const noexcept {
    return this->is_done();
}


template <typename T>
void coen::pinc::async<T>::await_suspend(
    std::coroutine_handle<> caller_handle
)
{
    m_caller_handle = std::move(caller_handle);        // this may break when I disable copy ctor for async
    this->run();
    if (mptr_self_handle->done()) {         // may need to add a special case for awaiting on generators
        m_caller_handle.resume();
    }
}


void coen::pinc::async<void>::await_suspend(
    std::coroutine_handle<> caller_handle
)
{
    m_caller_handle = std::move(caller_handle);        // this may break when I disable copy ctor for async
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
    mptr_self_handle->promise().cancel();
}


void coen::pinc::async<void>::cancel() const {
    mptr_self_handle->promise().cancel();
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
void coen::pinc::context<T>::return_value(const T& to_return) noexcept {
    std::cout << "copying ref" << std::endl;
    m_return_value = to_return;
}


template <typename T>
void coen::pinc::context<T>::return_value(T&& to_return) noexcept {
    std::cout << "moving" << std::endl;
    m_return_value = std::move(to_return);
}


void coen::pinc::context<void>::return_void() noexcept {
}


template <typename T>
T coen::pinc::context<T>::get_return_value() const {
    return m_return_value;
}


template <typename T>
void coen::pinc::context<T>::cancel() {
    // *(muptr_handle).destroy();
    muptr_handle.reset();
}


void coen::pinc::context<void>::cancel() {
    // *(muptr_handle).destroy();
    muptr_handle.reset();
}



class custom
{
    public:
    custom() {
        std::cout << "created custom" << std::endl;
    }

    custom(const custom& rhs) {
        std::cout << "called copy ctor" << std::endl;
    }

    custom(const custom&& rhs) {
        std::cout << "called move ctor" << std::endl;
    }

    custom& operator=(const custom& rhs) {
        std::cout << "called operator=" << std::endl;
        return *this;
    }
};
const int a = 56;




coen::pinc::async<custom> custom_node() {
    co_return custom();                                 // rvalue
}

coen::pinc::async<std::shared_ptr<int>> ptr_node() {
    std::cout << "ptrnode() - start" << std::endl;
    co_return std::make_shared<int>(a);                 // rvalue
    std::cout << "ptrnode() - exit" << std::endl;
}

coen::pinc::async<std::string> str_node() {
    std::cout << "strnode() - start" << std::endl;
    co_return "banana";                                 // rvalue
    std::cout << "strnode() - exit" << std::endl;
}

coen::pinc::async<int> node(int input) {
    std::cout << "node() - start" << std::endl;
    co_return 15 * input;                               // rvalue
    std::cout << "node() - exit" << std::endl;
}

coen::pinc::async<int> leaf() {
    std::cout << "leaf() - start" << std::endl;
    co_return 5;                                        // rvalue
    std::cout << "leaf() - exit" << std::endl;
}

coen::pinc::async<int> branch() {
    std::cout << "branch() - start" << std::endl;
    int response = co_await leaf();
    std::cout << "branch() - still" << std::endl;
    co_return response;                                 // lvalue
    std::cout << "branch() - exit" << std::endl;
}

coen::pinc::async<> root() {
    std::cout << "root() - start" << std::endl;
    int response = co_await branch() + co_await node(2);
    std::cout << "root() - still" << std::endl;
    std::cout << response << " " << co_await str_node() << " " << co_await ptr_node() << std::endl;
    custom obj = co_await custom_node();
    std::cout << "root() - exit" << std::endl;
}

int main() 
{
    std::cout << &a << std::endl;

    std::cout << "main() - start" << std::endl;
    auto coro = root();
    std::cout << "main() - still" << std::endl;
    coro.run();
    std::cout << "main() - exit" << std::endl;
}