/* -- COPYRIGHT AND LEGAL -- */

// preprocessor flags
#pragma once

#include <array>
#include <cstdint>
#include <coroutine>
#include <functional>
#include <optional>
#include <queue>
#include <thread>


namespace coen
{
    namespace pinc
    {
        // POSTULATES
        //
        // 1. Coroutine is a passive entity. When it is scheduled, it becomes a Task

        // ---------------------------------------------------------------------
        // TYPES
        // ---------------------------------------------------------------------

        // -- docs --
        template <
            typename t_TYPE = void,
            typename... t_ARGS
        >
        class awaiter;

        // template <>
        // class condition;

        // -- docs --
        template <
            typename t_TYPE = void,
            typename... t_ARGS
        >
        class coroutine;

        // -- docs --
        // class Event;

        // -- docs --
        class generator;

        // -- docs --
        template <
            typename t_TYPE = void,
            typename... t_ARGS
        >
        class Scheduler;

        // -- docs --
        // class ThreadPool;

        // ---------------------------------------------------------------------
        // EVENT-LOOP APIs
        // ---------------------------------------------------------------------

        // -- docs --
        template <typename t_TYPE, typename... t_ARGS>
        uint16_t start(
            coroutine<t_TYPE, t_ARGS...> coroutine,
            size_t poolsize = std::thread::hardware_concurrency()
        );

        // -- docs --
        int stop(
            bool force = true
        );

        // ---------------------------------------------------------------------
        // FUNCTIONAL APIs
        // ---------------------------------------------------------------------

        // -- docs --
        template <
            typename t_TYPE = void,
            typename... t_ARGS
        >
        int add_task(
            const coroutine<> &coroutine
        );

        // -- docs --
        template <
            typename t_TYPE = void,
            size_t t_SIZE = 0,
            typename... t_ARGS
        >
        coroutine<std::array<t_TYPE, t_SIZE>> gather(
            const std::array<coroutine<t_TYPE, t_ARGS...>, t_SIZE> coroutines
        );

        // -- docs --
        template <
            typename Rep,
            typename Period
        >
        coroutine<> sleep(
            const std::chrono::duration<Rep, Period> &duration
        );

        // -- docs --
        // bridge between sync to async code
        template <
            typename t_TYPE = void,
            typename... t_ARGS
        >
        coroutine<t_TYPE> to_thread(
            const std::function<void()> task,
            const bool lazy = true
        );

        // -- docs --
        // bridge between async to sync
        // immediately execute an awaitable blocking the caller thread seperate
        // from the core scheduler. This can be useful where a synchronous section
        // of a code requires to something from an asychronous code
        template <
            typename t_TYPE = void,
            typename... t_ARGS
        >
        t_TYPE sync(
            coroutine<t_TYPE, t_ARGS...> coroutine
        );
    }
}

// -----------------------------------------------------------------------------
// TYPES INTERFACE
// -----------------------------------------------------------------------------

template <
    typename t_TYPE, 
    typename... t_ARGS
>
class coen::pinc::coroutine
{
    public:

        // -- docs --
        class promise_type;

        // -- docs --
        using t_handle = std::coroutine_handle<promise_type>;

    public:

        // -- docs --
        uint16_t destroy();

        // -- docs --
        std::optional<t_TYPE> get_result() const;

        // -- docs --
        bool is_done();

        // -- docs --
        uint16_t resume();

        // -- docs --
        std::optional<t_TYPE> yield_next();

        // -- TEMPORARY --
        coroutine() {}

        // -- docs --
        ~coroutine() noexcept;

    private:

        // -- docs --
        bool i_is_resumable();
    
    private:

        // -- docs --
        explicit coroutine(t_handle handle);

    private:
        uint m_state;
        t_handle m_handle;
};


template <typename... t_ARGS>
class coen::pinc::coroutine<void, t_ARGS...>
{
    public:

        // -- docs --
        class promise_type;

        // -- docs --
        using t_handle = std::coroutine_handle<promise_type>;

    public:

        // -- docs --
        uint16_t destroy();

        // -- docs --
        bool is_done();

        // -- docs --
        uint16_t resume();

        // -- TEMPORARY --
        coroutine() {}

        // -- docs --
        ~coroutine() noexcept;

    private:

        // -- docs --
        bool i_is_resumable();

        // -- docs --
        explicit coroutine(t_handle handle);

    private:
        uint m_state;
        t_handle m_handle;
};


template <typename t_TYPE, typename... t_ARGS>
class coen::pinc::awaiter
{
    public:

        // -- docs --
        explicit awaiter(
            typename coen::pinc::coroutine<t_TYPE, t_ARGS...>::t_handle handle
        );

        // -- docs --
        bool await_ready();

        // -- docs --
        void await_suspend(
            std::coroutine_handle<> handle
            // coen::pinc::coroutine<t_TYPE, t_ARGS...>::t_handle handle
        );

        // -- docs --
        t_TYPE await_resume();

        // -- docs --
        // ~awaiter();

    private:

        typename coen::pinc::coroutine<t_TYPE, t_ARGS...>::t_handle m_calle;
        // coen::pinc::coroutine<t_TYPE, t_ARGS...> m_calle;
};


template <
    typename t_TYPE,
    typename... t_ARGS
>
class coen::pinc::coroutine<t_TYPE, t_ARGS...>::promise_type
{
    public:

        // -- docs --
        coen::pinc::coroutine<t_TYPE, t_ARGS...> get_return_object();
        
        // -- docs --
        std::suspend_always initial_suspend();
        
        // -- docs --
        std::suspend_always final_suspend() noexcept;
        
        // -- docs --
        template <typename t_TYPE_rhs, typename... t_ARGS_rhs>
        coen::pinc::awaiter<t_TYPE_rhs, t_ARGS_rhs...> await_transform(
            coen::pinc::coroutine<t_TYPE_rhs, t_ARGS_rhs...> rhs_coro
        );

        // -- docs --
        std::suspend_always yield_value(
            t_TYPE rhs_value
        );

        // -- docs --
        void return_value(
            t_TYPE rhs_value
        );

        // -- docs --
        void unhandled_exception() noexcept;

        // -- docs --
        std::optional<t_TYPE> get_return_value() const;

        // -- docs --
        std::optional<t_TYPE> get_yield_value() const;

    private:
        coen::pinc::coroutine<t_TYPE, t_ARGS...>::t_handle m_handle;
        
        // It is not guaranteed that the templated type would provide a default
        // constructor to promise. Thus we are required to use optional type for
        // providing response in case a result or yield value is requested before
        // they are provided to the coroutine
        t_TYPE m_return_value;
        t_TYPE m_yield_value;
};


template <typename... t_ARGS>
class coen::pinc::coroutine<void, t_ARGS...>::promise_type
{
    public:

        // -- docs --
        coen::pinc::coroutine<void, t_ARGS...> get_return_object();
        
        // -- docs --
        std::suspend_always initial_suspend();
        
        // -- docs --
        std::suspend_always final_suspend() noexcept;

        // -- docs --
        template <typename t_TYPE_rhs, typename... t_ARGS_rhs>
        coen::pinc::awaiter<t_TYPE_rhs, t_ARGS_rhs...> await_transform(
            coen::pinc::coroutine<t_TYPE_rhs, t_ARGS_rhs...> rhs_coro
        );

        // -- docs --
        
        // -- docs --
        void return_void();

        // -- docs --
        void unhandled_exception() noexcept;

    private:
        coen::pinc::coroutine<void, t_ARGS...>::t_handle m_handle;
};


template <typename t_TYPE, typename... t_ARGS>
class coen::pinc::Scheduler
{
    public:
    
    // -- docs --
    void start();

    // -- docs --
    void stop(bool apply_force = false);

    // -- docs --
    uint16_t add_task(
        coen::pinc::coroutine<t_TYPE, t_ARGS...> coroutine 
    );

    // -- EXPERIMENTAL --
    template <typename Rep, typename Period>
    uint16_t add_timer(
        const std::chrono::duration<Rep, Period> &duration,
        const coen::pinc::coroutine<t_TYPE, t_ARGS...> blocked_routine
    );
};


// -----------------------------------------------------------------------------
// IMPLEMENATIONS
// -----------------------------------------------------------------------------

template <typename t_TYPE, typename... t_ARGS>
coen::pinc::awaiter<t_TYPE, t_ARGS...>
::awaiter(
    typename coen::pinc::coroutine<t_TYPE, t_ARGS...>::t_handle handle
)
{
    m_calle = handle;
}


template <typename t_TYPE, typename... t_ARGS>
bool
coen::pinc::awaiter<t_TYPE, t_ARGS...>
::await_ready()
{
    return m_calle.done();
}


template <typename t_TYPE, typename... t_ARGS>
void
coen::pinc::awaiter<t_TYPE, t_ARGS...>
::await_suspend(
    std::coroutine_handle<> handle
)
{
    handle.resume();
}


template <typename t_TYPE, typename... t_ARGS>
t_TYPE
coen::pinc::awaiter<t_TYPE, t_ARGS...>
::await_resume()
{
    return m_calle.promise().get_return_value().value_or(-1);
}


template <typename t_TYPE, typename... t_ARGS>
coen::pinc::coroutine<t_TYPE, t_ARGS...>
::coroutine(
    coen::pinc::coroutine<t_TYPE, t_ARGS...>::t_handle handle
)
{
    m_handle = handle;
}


template <typename... t_ARGS>
coen::pinc::coroutine<void, t_ARGS...>
::coroutine(
    coen::pinc::coroutine<void, t_ARGS...>::t_handle handle
)
{
    m_handle = handle;
}


template <typename t_TYPE, typename... t_ARGS>
coen::pinc::coroutine<t_TYPE, t_ARGS...>
::~coroutine() noexcept
{
}


template <typename... t_ARGS>
coen::pinc::coroutine<void, t_ARGS...>
::~coroutine() noexcept
{
}


template <typename t_TYPE, typename... t_ARGS>
uint16_t
coen::pinc::coroutine<t_TYPE, t_ARGS...>
::destroy()
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


// VOID specialized
template <typename... t_ARGS>
uint16_t
coen::pinc::coroutine<void, t_ARGS...>
::destroy()
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


template <typename t_TYPE, typename... t_ARGS>
std::optional<t_TYPE>
coen::pinc::coroutine<t_TYPE, t_ARGS...>
::get_result() const
{
    return m_handle.promise().get_return_value();
}


template <typename t_TYPE, typename... t_ARGS>
bool 
coen::pinc::coroutine<t_TYPE, t_ARGS...>
::is_done()
{
    return m_handle.done();
}


// VOID specialized
template <typename... t_ARGS>
bool 
coen::pinc::coroutine<void, t_ARGS...>
::is_done()
{
    return m_handle.done();
}


template <typename t_TYPE, typename... t_ARGS>
uint16_t 
coen::pinc::coroutine<t_TYPE, t_ARGS...>
::resume()
{
    if (i_is_resumable())
    {
        m_handle.resume();
        return 0;
    }
    return 1;
}


// type VOID specialized
template <typename... t_ARGS>
uint16_t
coen::pinc::coroutine<void, t_ARGS...>
::resume()
{
    if (i_is_resumable())
    {
        m_handle.resume();
        return 0;
    }
    return 1;
}


template <typename t_TYPE, typename... t_ARGS>
std::optional<t_TYPE>
coen::pinc::coroutine<t_TYPE, t_ARGS...>
::yield_next()
{
    resume();
    return m_handle.promise().get_yield_value();
}


template <typename t_TYPE, typename... t_ARGS>
bool 
coen::pinc::coroutine<t_TYPE, t_ARGS...>
::i_is_resumable()
{
    return !is_done();   
}


// type VOID specialized
template <typename... t_ARGS>
bool
coen::pinc::coroutine<void, t_ARGS...>
::i_is_resumable()
{
    return !is_done();
}


template <typename t_TYPE, typename... t_ARGS>
coen::pinc::coroutine<t_TYPE, t_ARGS...>
coen::pinc::coroutine<t_TYPE, t_ARGS...>::promise_type
::get_return_object()
{
    m_handle = pinc::coroutine<t_TYPE, t_ARGS...>::t_handle::from_promise(*this);
    return pinc::coroutine<t_TYPE, t_ARGS...>(m_handle);
}


// type VOID specialized
template <typename... t_ARGS>
coen::pinc::coroutine<void, t_ARGS...>
coen::pinc::coroutine<void, t_ARGS...>::promise_type
::get_return_object()
{
    m_handle = pinc::coroutine<void, t_ARGS...>::t_handle::from_promise(*this);
    return pinc::coroutine<void, t_ARGS...>(m_handle);
}


template <typename t_TYPE, typename... t_ARGS>
std::suspend_always 
coen::pinc::coroutine<t_TYPE, t_ARGS...>::promise_type
::initial_suspend()
{
    return {};
}


// type VOID specialized
template <typename... t_ARGS>
std::suspend_always 
coen::pinc::coroutine<void, t_ARGS...>::promise_type
::initial_suspend()
{
    return {};
}


template <typename t_TYPE, typename... t_ARGS>
std::suspend_always 
coen::pinc::coroutine<t_TYPE, t_ARGS...>::promise_type
::final_suspend() noexcept
{
    return {};
}


// type VOID specialized
template <typename... t_ARGS>
std::suspend_always 
coen::pinc::coroutine<void, t_ARGS...>::promise_type
::final_suspend() noexcept
{
    return {};
}


template <typename t_TYPE, typename... t_ARGS>
template <typename t_TYPE_rhs, typename... t_ARGS_rhs>
coen::pinc::awaiter<t_TYPE_rhs, t_ARGS_rhs...>
coen::pinc::coroutine<t_TYPE, t_ARGS...>::promise_type
::await_transform(
    coen::pinc::coroutine<t_TYPE_rhs, t_ARGS_rhs...> rhs_coro
)
{
    rhs_coro.resume();
    return coen::pinc::awaiter<t_TYPE_rhs, t_ARGS_rhs...>(
        rhs_coro.m_handle
    );
}


template <typename... t_ARGS>
template <typename t_TYPE_rhs, typename... t_ARGS_rhs>
coen::pinc::awaiter<t_TYPE_rhs, t_ARGS_rhs...>
coen::pinc::coroutine<void, t_ARGS...>::promise_type
::await_transform(
    coen::pinc::coroutine<t_TYPE_rhs, t_ARGS_rhs...> rhs_coro
)
{
    rhs_coro.resume();
    return coen::pinc::awaiter<t_TYPE_rhs, t_ARGS_rhs...>(
        rhs_coro.m_handle
    );
}


template <typename t_TYPE, typename... t_ARGS>
std::suspend_always 
coen::pinc::coroutine<t_TYPE, t_ARGS...>::promise_type
::yield_value(
    t_TYPE rhs_value
)
{
    m_yield_value = rhs_value;
    return {};
}



template <typename... t_ARGS>
void 
coen::pinc::coroutine<void, t_ARGS...>::promise_type
::return_void()
{
}


template <typename t_TYPE, typename... t_ARGS>
void
coen::pinc::coroutine<t_TYPE, t_ARGS...>::promise_type
::return_value(
    t_TYPE rhs_value
)
{
    m_return_value = rhs_value;
}


template <typename t_TYPE, typename... t_ARGS>
void 
coen::pinc::coroutine<t_TYPE, t_ARGS...>::promise_type
::unhandled_exception() noexcept
{
}


// type VOID specialized
template <typename... t_ARGS>
void
coen::pinc::coroutine<void, t_ARGS...>::promise_type
::unhandled_exception() noexcept
{
}


template <typename t_TYPE, typename... t_ARGS>
std::optional<t_TYPE>
coen::pinc::coroutine<t_TYPE, t_ARGS...>::promise_type
::get_return_value() const
{
    return m_return_value;
}


template <typename t_TYPE, typename... t_ARGS>
std::optional<t_TYPE>
coen::pinc::coroutine<t_TYPE, t_ARGS...>::promise_type
::get_yield_value() const
{
    return m_yield_value;
}


// -- TODO -- Implement Scheduler Here


template <
    typename t_TYPE,
    typename... t_ARGS
>
uint16_t coen::pinc::start(
    coen::pinc::coroutine<t_TYPE, t_ARGS...> coroutine,
    size_t pool_size 
)
{
    coroutine.resume();
    return 0;
}