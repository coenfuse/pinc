#include <coroutine>
#include <chrono>
#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

namespace coen {
namespace pinc {

template <typename T = void>
class Awaitable {
public:
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        T result;
        std::exception_ptr exception;

        Awaitable get_return_object() { return Awaitable(handle_type::from_promise(*this)); }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_value(T value) { result = value; }
        void unhandled_exception() { exception = std::current_exception(); }
    };

    Awaitable(handle_type h) : handle(h) {}
    Awaitable(Awaitable&& other) noexcept : handle(other.handle) { other.handle = nullptr; }
    ~Awaitable() { if (handle) handle.destroy(); }

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> h) {
        // Schedule both the current coroutine and the awaiting coroutine
        Pool::get_instance().run_task([this] { handle.resume(); });
        Pool::get_instance().run_task([h] { h.resume(); });
    }
    T await_resume() {
        if (handle.promise().exception) {
            std::rethrow_exception(handle.promise().exception);
        }
        return handle.promise().result;
    }

    handle_type handle;
};

template <>
class Awaitable<void> {
public:
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        std::exception_ptr exception;

        Awaitable get_return_object() { return Awaitable(handle_type::from_promise(*this)); }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { exception = std::current_exception(); }
    };

    Awaitable(handle_type h) : handle(h) {}
    Awaitable(Awaitable&& other) noexcept : handle(other.handle) { other.handle = nullptr; }
    ~Awaitable() { if (handle) handle.destroy(); }

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> h) {
        Pool::get_instance().run_task([this] { handle.resume(); });
        Pool::get_instance().run_task([h] { h.resume(); });
    }
    void await_resume() {
        if (handle.promise().exception) {
            std::rethrow_exception(handle.promise().exception);
        }
    }

    handle_type handle;
};

class Pool {
public:
    static Pool& get_instance() {
        static Pool instance;
        return instance;
    }

    void start(size_t thread_count = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < thread_count; ++i) {
            threads.emplace_back([this] { run(); });
        }
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            stop_flag = true;
        }
        condition.notify_all();
        for (auto& thread : threads) {
            thread.join();
        }
    }

    void run_task(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            tasks.push(std::move(task));
        }
        condition.notify_one();
    }

private:
    Pool() = default;
    Pool(const Pool&) = delete;
    Pool& operator=(const Pool&) = delete;

    void run() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                condition.wait(lock, [this] { return stop_flag || !tasks.empty(); });
                if (stop_flag && tasks.empty()) {
                    return;
                }
                task = std::move(tasks.front());
                tasks.pop();
            }
            task();
        }
    }

    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop_flag = false;
};

template <typename T = void>
T await(Awaitable<T> awaitable) {
    return co_await awaitable;
}

template <typename T = void>
Awaitable<void> gather(const std::vector<Awaitable<T>>& awaitables) {
    for (auto& awaitable : awaitables) {
        co_await awaitable;
    }
}

Awaitable<void> sleep(std::chrono::seconds duration) {
    co_await std::suspend_always{};
    std::this_thread::sleep_for(duration);
}

int start(Awaitable<void> awaitable, size_t pool_size = std::thread::hardware_concurrency()) {
    Pool::get_instance().start(pool_size);
    Pool::get_instance().run_task([&] { awaitable.handle.resume(); });
    Pool::get_instance().stop();
    return 0;
}

int stop(bool force = true) {
    Pool::get_instance().stop();
    return 0;
}

} // namespace pinc
} // namespace coen

// User code
coen::pinc::Awaitable<void> coro_loop(const size_t length) {
    for (size_t itr = 0; itr <= length; itr++) {
        std::cout << "coro[" << length << "] -> itr " << itr << std::endl;
        co_await coen::pinc::sleep(std::chrono::seconds(1));
    }
}

coen::pinc::Awaitable<void> root() {
    std::vector<coen::pinc::Awaitable<void>> tasks;
    for (int i = 0; i < 3; i++) {
        tasks.push_back(coro_loop(i * i));
    }
    co_await coen::pinc::gather(tasks);
}

int main() {
    return coen::pinc::start(root());
}