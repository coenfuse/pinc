/* -- COPYRIGHT AND LEGAL -- */

// preprocessor flags
#pragma once

#include <array>
#include <coroutine>
#include <functional>
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
        // template <
        //     typename t_TYPE = void, 
        //     typename... t_ARGS
        // >
        // class awaiter;

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
        // template <
        //     typename t_TYPE = void, 
        //     typename... t_ARGS
        // >
        // class Scheduler;

        // -- docs --
        // class ThreadPool;

        // ---------------------------------------------------------------------
        // EVENT-LOOP APIs
        // ---------------------------------------------------------------------

        // -- docs --
        int start(
            const coroutine<> &coroutine,
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
        using t_handle = std::coroutine_handle<promise_type>; // <t_TYPE, t_ARGS...>>;

    public:

        // -- docs --
        uint resume();

        // -- docs --
        bool done();

        // -- docs --
        uint destroy();

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


template <
    typename t_TYPE,
    // typename t_TYPE_rhs,
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
        // template <typename t_TYPE_rv, typename... t_ARGS_rv>
        // std::suspend_always await_transform(
        //     coen::pinc::coroutine<t_TYPE_rv, t_ARGS_rv> rhs_coro
        // );

        // -- docs --
        std::suspend_always await_transform(
            coen::pinc::coroutine<t_TYPE, t_ARGS...> rhs_coro
        );

        // -- docs --
        std::suspend_always yield_value(
            t_TYPE rhs_value
        );
        
        // -- docs --
        void return_void();

        // -- docs --
        void unhandled_exception() noexcept;

    private:
        coen::pinc::coroutine<t_TYPE, t_ARGS...>::t_handle m_handle;
};





// -----------------------------------------------------------------------------
// IMPLEMENATIONS
// -----------------------------------------------------------------------------

template <typename t_TYPE, typename... t_ARGS>
coen::pinc::coroutine<t_TYPE, t_ARGS...>::coroutine(
    coen::pinc::coroutine<t_TYPE, t_ARGS...>::t_handle handle
)
{
    m_handle = handle;
}


template <typename t_TYPE, typename... t_ARGS>
coen::pinc::coroutine<t_TYPE, t_ARGS...>::~coroutine() noexcept
{
}


template <typename t_TYPE, typename... t_ARGS>
uint coen::pinc::coroutine<t_TYPE, t_ARGS...>::destroy()
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
bool coen::pinc::coroutine<t_TYPE, t_ARGS...>::done()
{
    return m_handle.done();
}


template <typename t_TYPE, typename... t_ARGS>
uint coen::pinc::coroutine<t_TYPE, t_ARGS...>::resume()
{
    if (i_is_resumable())
    {
        m_handle.resume();
        return 0;
    }
    return 1;
}


template <typename t_TYPE, typename... t_ARGS>
bool coen::pinc::coroutine<t_TYPE, t_ARGS...>::i_is_resumable()
{
    return !done();   
}


template <typename t_TYPE, typename... t_ARGS>
coen::pinc::coroutine<t_TYPE, t_ARGS...> coen::pinc::coroutine<t_TYPE, t_ARGS...>::promise_type::get_return_object()
{
    m_handle = pinc::coroutine<t_TYPE, t_ARGS...>::t_handle::from_promise(*this);
    return pinc::coroutine<t_TYPE, t_ARGS...>(m_handle);
}


template <typename t_TYPE, typename... t_ARGS>
std::suspend_always coen::pinc::coroutine<t_TYPE, t_ARGS...>::promise_type::initial_suspend()
{
    return {};
}


template <typename t_TYPE, typename... t_ARGS>
std::suspend_always coen::pinc::coroutine<t_TYPE, t_ARGS...>::promise_type::final_suspend() noexcept
{
    return {};
}


template <typename t_TYPE, typename... t_ARGS>
std::suspend_always coen::pinc::coroutine<t_TYPE, t_ARGS...>::promise_type::await_transform(
    coen::pinc::coroutine<t_TYPE, t_ARGS...> rhs_coro
)
{
    return {};
}


template <typename t_TYPE, typename... t_ARGS>
std::suspend_always coen::pinc::coroutine<t_TYPE, t_ARGS...>::promise_type::yield_value(
    t_TYPE rhs_value
)
{
    return {};
}


template <typename t_TYPE, typename... t_ARGS>
void coen::pinc::coroutine<t_TYPE, t_ARGS...>::promise_type::return_void()
{
}


template <typename t_TYPE, typename... t_ARGS>
void coen::pinc::coroutine<t_TYPE, t_ARGS...>::promise_type::unhandled_exception() noexcept
{
}