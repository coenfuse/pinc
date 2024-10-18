#include <coroutine>        // std::coroutine_handle
#include <exception>
#include <iostream>         
#include <memory>           // std::smart_ptr
#include <string>
#include <utility>          // std::exchange




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

        async(const async<T>& other) = delete;
        async(async<T>&& other) noexcept;
        async& operator=(const async<T>& other) = delete;
        async& operator=(async&& other) noexcept;
        ~async() noexcept;

    public:

        bool await_ready() const noexcept;
        void await_suspend(std::coroutine_handle<> caller_handle);
        T await_resume() const;

        void run() const;       // scheduled to be privatised and only exposed to scheduler
        bool is_done() const;   // ..

        T get_result() const;   // temporary, remove

    private:

        explicit async(coro_handle handle);

    private:

        std::coroutine_handle<> m_caller_handle;
        coro_handle m_handle;
};



template <>
class coen::pinc::async<void>
{
    public:

        friend class context<>;
        using promise_type = class context<>;
        using coro_handle  = std::coroutine_handle<promise_type>;
    
        async(const async<void>& other) = delete;
        async(async<void>&& other) noexcept;
        async& operator=(const async<void>& other) = delete;
        async& operator=(async&& other) noexcept;
        ~async() noexcept;

    public:

        bool await_ready() const noexcept;
        void await_suspend(std::coroutine_handle<> caller_handle);
        void await_resume() const;
    
        void run() const;
        bool is_done() const;


    private:

        explicit async(coro_handle handle);

    private:

        std::coroutine_handle<> m_caller_handle;
        coro_handle m_handle;
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
        std::suspend_always initial_suspend() noexcept;
        std::suspend_always final_suspend() noexcept;
        static void unhandled_exception() noexcept;
        void return_value(const T& to_return) noexcept;
        void return_value(T&& to_return) noexcept;

        T get_return_value() const;

    private:

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
        std::suspend_always initial_suspend() noexcept;
        std::suspend_always final_suspend() noexcept;
        static void unhandled_exception() noexcept;
        void return_void() noexcept;
};




template <typename T>
coen::pinc::async<T>::async(coro_handle handle) {
    m_handle = std::move(handle);
}


template <typename T>
coen::pinc::async<T>::async(async<T>&& other) noexcept 
{
    m_handle = std::exchange(other.m_handle, {});
    m_caller_handle = std::exchange(other.m_caller_handle, {});
}


template <typename T>
coen::pinc::async<T>& coen::pinc::async<T>::operator=(async<T>&& other) noexcept
{
    if (this != &other) 
    {
        if (m_handle) { m_handle.destroy(); }               // unsafe
        m_handle = std::exchange(other.m_handle, {});
        m_caller_handle = std::exchange(other.m_caller_handle, {});
    }
    return *this;
}


coen::pinc::async<void>::async(coro_handle handle) {
    m_handle = std::move(handle);
}


coen::pinc::async<void>::async(async<void>&& other) noexcept
{
    m_handle = std::exchange(other.m_handle, {});
    m_caller_handle = std::exchange(other.m_caller_handle, {});
}


coen::pinc::async<void>& coen::pinc::async<void>::operator=(async<void>&& other) noexcept
{
    if (this != &other)
    {
        if (m_handle) { m_handle.destroy(); }               // unsafe
        m_handle = std::exchange(other.m_handle, {});
        m_caller_handle = std::exchange(other.m_caller_handle, {});
    }
    return *this;
}


template <typename T>
coen::pinc::async<T>::~async() noexcept {
    if (is_done()) {
        std::cout << "[WARN] -- PINC -- destroying an unfinished coroutine returning " << m_handle.promise().m_return_value << std::endl;
    }
    m_handle.destroy();
}


coen::pinc::async<void>::~async() noexcept {
    if (is_done()) {
        std::cout << "[WARN] -- PINC -- destroying an unfinished coroutine returinig void" << std::endl;
    }
    m_handle.destroy();
}


template <typename T>
bool coen::pinc::async<T>::await_ready() const noexcept {
    return is_done();
}


bool coen::pinc::async<void>::await_ready() const noexcept {
    return is_done();
}


template <typename T>
void coen::pinc::async<T>::await_suspend(
    std::coroutine_handle<> caller_handle
)
{
    m_caller_handle = std::move(caller_handle);
    run();
    if (is_done()) {         // may need to add a special case for awaiting on generators
        m_caller_handle.resume();
    }
}


void coen::pinc::async<void>::await_suspend(
    std::coroutine_handle<> caller_handle
)
{
    m_caller_handle = std::move(caller_handle);
    run();
    if (is_done()) {         // may need to add a special case for awaiting on generators
        m_caller_handle.resume();
    }
}


template <typename T>
T coen::pinc::async<T>::await_resume() const {
    return m_handle.promise().get_return_value();
}


void coen::pinc::async<void>::await_resume() const {
}


template <typename T>
void coen::pinc::async<T>::run() const {
    if (!is_done()) {
        m_handle.resume();
    }
}


void coen::pinc::async<void>::run() const {
    if (!is_done()) {
        m_handle.resume();
    }
}


template <typename T>
bool coen::pinc::async<T>::is_done() const {
    return m_handle.done();
}


bool coen::pinc::async<void>::is_done() const {
    return m_handle.done();
}


template <typename T>
T coen::pinc::async<T>::get_result() const {
    return await_resume();
}


template <typename T>
coen::pinc::async<T> coen::pinc::context<T>::get_return_object() {
    return async<T>(coro_handle::from_promise(*this));
}


coen::pinc::async<void> coen::pinc::context<void>::get_return_object() {
    return async<void>(coro_handle::from_promise(*this));
}


template <typename T>
std::suspend_always coen::pinc::context<T>::initial_suspend() noexcept {
    return {};
}


std::suspend_always coen::pinc::context<void>::initial_suspend() noexcept {
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
    std::terminate();
}


void coen::pinc::context<void>::unhandled_exception() noexcept {
    std::terminate();
}


template <typename T>
void coen::pinc::context<T>::return_value(const T& to_return) noexcept {
    m_return_value = to_return;
}


template <typename T>
void coen::pinc::context<T>::return_value(T&& to_return) noexcept {
    m_return_value = std::move(to_return);
}


void coen::pinc::context<void>::return_void() noexcept {
}


template <typename T>
T coen::pinc::context<T>::get_return_value() const {
    return m_return_value;
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


// coen::pinc::async<custom> custom_node() {
//     co_return custom();                                 // rvalue
// }
// 
// coen::pinc::async<std::shared_ptr<int>> ptr_node() {
//     std::cout << "ptrnode() - start" << std::endl;
//     co_return std::make_shared<int>(a);                 // rvalue
//     std::cout << "ptrnode() - exit" << std::endl;
// }

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
    std::cout << response << " " << co_await str_node() << " " << std::endl; // co_await ptr_node() << std::endl;
    // custom obj = co_await custom_node();
    std::cout << "root() - exit" << std::endl;
    co_return;
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