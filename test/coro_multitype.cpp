#include <coroutine>        // std::coroutine_handle
#include <exception>
#include <iostream>         
#include <memory>           // std::smart_ptr
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <utility>          // std::exchange




namespace coen 
{
    namespace pinc
    {
        // ---------------------------------------------------------------------
        // module types
        // ---------------------------------------------------------------------

        class async_base;

        // -- docs --
        template <typename T = void>
        class async;

        // -- docs --
        template <typename T = void>
        class context;

        // -- docs --
        // template <typename T>
        // class generator;

        // -- docs --
        // template <typename T = void>
        class scheduler;

        // ---------------------------------------------------------------------
        // event loop APIs
        // ---------------------------------------------------------------------

        // -- docs --
        uint16_t start(
            std::unique_ptr<async_base> task,
            size_t poolsize = std::thread::hardware_concurrency()
        );

        // -- docs --
        // uint16_t stop(
        //     bool force = true
        // );

    } // namespace pinc

} // namespace coen



class coen::pinc::async_base
{
    public:

        virtual bool await_ready() const noexcept = 0;
        virtual void await_suspend(std::coroutine_handle<> caller_handle) = 0;
        virtual void run() const = 0;
        virtual bool is_done() const = 0;
};




template <typename T>
class coen::pinc::async : public coen::pinc::async_base
{
    public:
        
        friend class context<T>;
        using promise_type = class context<T>;
        using coro_handle  = std::coroutine_handle<promise_type>;
        // using value_type   = T;

        async(const async<T>& other) = delete;
        async(async<T>&& other) noexcept;
        async& operator=(const async<T>& other) = delete;
        async& operator=(async&& other) noexcept;
        ~async() noexcept;

    public:

        bool await_ready() const noexcept override;
        void await_suspend(std::coroutine_handle<> caller_handle) override;
        T await_resume() const;

        void run() const override;       // scheduled to be privatised and only exposed to scheduler
        bool is_done() const override;   // ..

        T get_result() const;            // temporary, remove

    private:

        explicit async(coro_handle handle);

    private:

        std::coroutine_handle<> m_caller_handle;
        coro_handle m_handle;
};



template <>
class coen::pinc::async<void> : public coen::pinc::async_base
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

        bool await_ready() const noexcept override;
        void await_suspend(std::coroutine_handle<> caller_handle) override;
        void await_resume() const;
    
        void run() const override;          // scheduled to be privatised and only exposed to scheduler
        bool is_done() const override;      // ..

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



// pending thought on whether generator can await on a coroutine or not since if
// it does then it must be bound to the scheduler. And if it is, then it yield
// support and generators in general should be implemented within async<T>::context
// Refer to example, https://en.cppreference.com/w/cpp/coroutine/coroutine_handle
/*
template <typename T>
class coen::pinc::generator
{
    public:

    private:

    class context;
    class iterator;
};



template <typename T>
class coen::pinc::generator<T>::context
{
    public:

        friend class generator<T>;
        using coro_handle = typename generator<T>::coro_handle;

        // ctors and dtors

    public:

        generator<T> get_return_object();
        static std::suspend_always initial_suspend() noexcept;
        static std::suspend_always final_suspend() noexcept;
        static void unhandled_exception() noexcept;
        std::suspend_always yield_value(T value) noexcept;

    private:

        std::optional<T> m_current_value;
};



class iterator
{
};
*/



class coen::pinc::scheduler
{
    public:

        explicit scheduler() noexcept;
        explicit scheduler(const scheduler& context) = delete;
        explicit scheduler(const scheduler&& context) = delete;  
        ~scheduler() noexcept;

    public:
    
        void add_task(std::unique_ptr<async_base> task);
        void add_timer();
        void start();
        void stop();

    private:

        std::queue<std::unique_ptr<async_base>> mptr_tasks;
        bool m_interrupt = true;
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


coen::pinc::scheduler::scheduler() noexcept {
}


coen::pinc::scheduler::~scheduler() noexcept {
}


void coen::pinc::scheduler::add_task(std::unique_ptr<async_base> task) {
    mptr_tasks.push(std::move(task));
}


void coen::pinc::scheduler::add_timer() {
}


void coen::pinc::scheduler::start() {
    m_interrupt = false;
    while (!m_interrupt && !mptr_tasks.empty()) {
        try {
            auto& task = mptr_tasks.front();
            if (!task->is_done()) {
                task->run();
            }
            else {
                mptr_tasks.pop();
            }
        }
        catch (...) {
        }
    }
}


void coen::pinc::scheduler::stop() {
    m_interrupt = false;
}


uint16_t coen::pinc::start(
    std::unique_ptr<async_base> task,
    size_t pool_size
)
{
    scheduler evloop = scheduler();
    evloop.add_task(std::move(task));
    evloop.start();
    return 0;
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
    coen::pinc::start(std::make_unique<coen::pinc::async<>>(root()));

    // std::cout << &a << std::endl;
    // std::cout << "main() - start" << std::endl;
    // auto coro = root();
    // std::cout << "main() - still" << std::endl;
    // coro.run();
    // std::cout << "main() - exit" << std::endl;
}