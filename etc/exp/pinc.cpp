#include <iostream>
#include <coroutine>
#include <thread>
#include <chrono>

// Awaitable type to represent a sleep operation in C++
struct Sleep {
    std::chrono::milliseconds duration;

    bool await_ready() const noexcept { return duration.count() == 0; }
    void await_suspend(std::coroutine_handle<> h) const {
        std::thread([h, duration = duration]() {
            std::this_thread::sleep_for(duration);
            h.resume();
        }).detach();
    }
    void await_resume() const noexcept {}
};

// Wrapper to make coroutine_handle awaitable
struct AwaitableCoroutine {
    std::coroutine_handle<> handle;

    bool await_ready() const noexcept { return !handle || handle.done(); }
    void await_suspend(std::coroutine_handle<>) const noexcept { handle.resume(); }
    void await_resume() const noexcept {}
};

struct Task {
    struct promise_type {
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() noexcept {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    Task(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Task() {
        if (handle) handle.destroy();
    }

    void start() {
        handle.resume();
    }

    AwaitableCoroutine operator co_await() const noexcept {
        return AwaitableCoroutine{handle};
    }
};

Task foo() {
    for (int i = 0; i < 15; ++i) {
        std::cout << "foo itr - " << i << std::endl;
        co_await Sleep{std::chrono::seconds(1)};
    }
}

Task bar() {
    for (int i = 0; i < 10; ++i) {
        std::cout << "bar itr - " << i << std::endl;
        co_await Sleep{std::chrono::seconds(2)};
    }
}

Task mainTask() {
    Task task1 = foo();
    Task task2 = bar();

    task1.start();
    task2.start();

    co_await task1;
    co_await task2;
}

int main() {
    mainTask().start();
    std::this_thread::sleep_for(std::chrono::seconds(20));  // Wait for coroutines to finish
    return 0;
}
