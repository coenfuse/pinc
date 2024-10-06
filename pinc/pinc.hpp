// preprocessor directives
#pragma once

// standard imports
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <coroutine>
#include <cstdint>
#include <deque>
#include <functional>
#include <initializer_list>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>
#include <vector>


namespace coen
{
    namespace pinc
    {
        // ---------------------------------------------------------------------
        // Types declaration
        // ---------------------------------------------------------------------

        // TODO : description
        template <typename t_TYPE = void, typename... t_ARGS>
        class Awaitable;

        // TODO : description
        template <typename... t_ARGS>
        class Condition;

        // TODO : description
        class Event;

        // TODO : description
        // If you add a timeout to future, it behaves as a timed awaitable
        // If you don't add a timeout, it behaves as a promise that promises you
        // to return a response in future, but not sure when
        template <typename t_TYPE = void>
        class Future;

        // TODO : description
        class Pool;

        // TODO : description
        template <typename t_TYPE, typename... t_ARGS>
        class Scheduler;

        // TODO : description
        class Task;

        // TODO : description
        class Timer;

        // ---------------------------------------------------------------------
        // PINC runtime APIs
        // ---------------------------------------------------------------------

        // TODO : description
        int start(
            const Awaitable<> awaitable,
            size_t poolsize = std::thread::hardware_concurrency()
        );

        // TODO : description
        int stop(
            bool force = true
        );

        // ---------------------------------------------------------------------
        // PINC behavioral APIs
        // ---------------------------------------------------------------------

        // TODO : description
        template <typename t_TYPE = void, typename... t_ARGS>
        t_TYPE await(
            Awaitable<t_TYPE, t_ARGS...> awaitable
        );

        // TODO : description
        template <typename t_TYPE = void, typename... t_ARGS>
        int add_task(
            Awaitable<t_TYPE, t_ARGS...> awaitable
        );

        // TODO : description
        template <typename t_TYPE = void, typename... t_ARGS>
        Awaitable<> gather(
            const std::vector<Awaitable<t_TYPE, t_ARGS...>> awaitables
        );

        template <typename t_TYPE = void, typename... t_ARGS>
        Awaitable<> gather(
            const std::initializer_list<Awaitable<t_TYPE, t_ARGS...>> awaitables
        );

        // TODO : description
        template <typename Rep, typename Period>
        Awaitable<> sleep(
            const std::chrono::duration<Rep, Period>& duration
        );

        // TODO : description
        template <typename t_TYPE = void, typename... t_ARGS>
        Awaitable<t_TYPE> to_thread(
            const std::function<t_TYPE(t_ARGS...)> task,
            const bool lazy = true
        );

        // wait until a confirmed timeout in future to execute the awaitable
	    // TODO : consider it implementing via wait_for() for simplicity
        template <typename t_TYPE = void, typename... t_ARGS, typename Rep, typename Period>
        Awaitable<> wait_until(
            Awaitable<t_TYPE, t_ARGS...> awaitable,
            const std::chrono::duration<Rep, Period>& duration
        );

	    // wait until an unconfirmed event in future to execute the awaitable
	    template <typename t_TYPE = void, typename... t_ARGS>
	    Awaitable<> wait_for(
		    Awaitable<t_TYPE, t_ARGS...> awaitable,
		    const Event& event
	    );

        // wait until an unconfirmed condition to become true to execute the awaitable
        template <typename t_TYPE = void, typename... t_ARGS, typename... tc_ARGS>
        Awaitable<> wait_for(
            Awaitable<t_TYPE, t_ARGS ...> awaitable,
            const Condition<tc_ARGS...>& condition
        );

        // immediately execute an awaitable blocking the caller thread seperate
        // from the core scheduler. This can be useful where a synchronous section
        // of a code requires to something from an asychronous code
        template <typename t_TYPE = void, typename... t_ARGS>
        t_TYPE sync(
            Awaitable<t_TYPE, t_ARGS...> awaitable
        );

        // ---------------------------------------------------------------------
        // PINC internal vars
        // ---------------------------------------------------------------------

        // Scheduler i_scheduler;
        Pool i_pool;
    };
};


// -----------------------------------------------------------------------------
// Types Interface Declaration
// -----------------------------------------------------------------------------

template <typename t_TYPE, typename... t_ARGS>
class coen::pinc::Awaitable
{
    public:
        Awaitable();
        Awaitable(std::function<t_TYPE(t_ARGS...)>);
        ~Awaitable();

    public:
        t_TYPE get_result() const;
        void execute();
        void suspend();
        void destroy();

    private:
        coen::pinc::Future<t_TYPE> m_future;
        std::tuple<t_ARGS...> m_params;
        std::function<t_TYPE(t_ARGS...)> m_handle;
};


template <typename... t_ARGS>
class coen::pinc::Condition
{
    public:
        Condition(const std::function<t_TYPE<t_ARGS...>> predicate);
        ~Condition();

    public:
        Awaitable<> wait();
        void resolve();                                                         // forceful bypass that could be useful during shutdown
        
    private:
        std::function<bool<t_ARGS...>> m_predicate;
};



class coen::pinc::Event
{
    public:
        Event();
        ~Event();

    public:
        void set();
        bool is_set();
        void clear();
        Awaitable<> wait();

    private:
        std::atomic_bool m_val;
};



template <typename t_TYPE>
class coen::pinc::Future
{
    public:
        Future();
        ~Future();

    public:
        bool is_available() const;
        void set_value(const t_TYPE value);
        void set_exception(const std::exception error);
        t_TYPE get_value() const;

    private:
        std::atomic_bool m_is_available = false;
        std::exception m_error;
        t_TYPE m_value;
};



class coen::pinc::Pool
{
    public:
        Pool(const size_t size = 1);
        void start();
        void stop(bool force = false);
        void run_task(const coen::pinc::Task& task, bool lazy = true);

    private:
        void __runtime(const size_t thread_id);

    private:
        size_t m_size;
        std::atomic_bool m_interrupt;
        std::atomic_uint16_t m_last_used_thread_id;

        std::vector<std::thread> m_pool;
        std::vector<std::deque<coen::pinc::Task>> m_jobs;
        std::vector<
            std::pair<
                std::unique_ptr<std::mutex>,
                std::unique_ptr<std::condition_variable>
            >
        > m_mxcv;
};



template <typename t_TYPE, typename... t_ARGS>
class coen::pinc::Scheduler
{
    public:
        Scheduler();
        ~Scheduler();

    public:
        void start();
        void stop(bool force = false);
        void add_task();
        void add_timer();
        void add_event();

    private:
        void __runtime();

    private:
        std::vector<Awaitable<>> m_event_queue;
        std::vector<Awaitable<>> m_timer_queue;
        std::deque<Awaitable<t_TYPE, t_ARGS...> m_task_queue;
};



class coen::pinc::Task
{
    public:
        Task() {}
        Task(std::function<void(const uint16_t)>, const uint16_t);
        void execute() const;

    private:
        std::function<void(const uint16_t)> m_job;
        uint16_t m_param;
};



class coen::pinc::Timer
{};



// -----------------------------------------------------------------------------
// Method Implementations
// -----------------------------------------------------------------------------