#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <pthread.h>

// -----------------------------------------------------------------------------
// interface
// -----------------------------------------------------------------------------
class Task
{
    public:
        Task(){}
        Task(std::function<void(const uint16_t)>, const uint16_t);
        void execute() const;

    private:
        std::function<void(const uint16_t)> m_job;
        uint16_t m_param;
};

Task::Task(std::function<void(const uint16_t)> job, const uint16_t param)
{
    m_job = job;
    m_param = param;
}

// later consider,
// try-catch for reliability
// future/promise for returns
// kwargs for variadic params
// overload for lambdas, fptrs, std::fuction, etc
void Task::execute() const {
    m_job(m_param);
}



class ThreadPool
{
    public:
        ThreadPool(const size_t size = 1);
        void start();
        void stop(bool force = false);
        void run_task(const Task& task);

    private:
        void __runtime(const size_t thread_id);

    private:
        size_t m_size;
        std::atomic_bool m_interrupt;
        std::atomic_uint16_t m_last_used_thread_id;

        std::vector<std::thread> m_pool;
        std::vector<std::deque<Task>> m_jobs;
        std::vector<
            std::pair<
                std::unique_ptr<std::mutex>,
                std::unique_ptr<std::condition_variable>
            >
        > m_mxcv;
};

ThreadPool::ThreadPool(const size_t size)
{
    m_size = size;
    m_last_used_thread_id.store(0);

    m_jobs.reserve(size);
    m_pool.reserve(size);
    m_mxcv.reserve(size);

    for (size_t index = 0; index < size; index++)
    {
        m_mxcv.push_back({
            std::make_unique<std::mutex>(),
            std::make_unique<std::condition_variable>()
        });
        m_jobs.push_back(std::deque<Task>());
    }
}

void ThreadPool::start()
{
    m_interrupt.store(false);
    for (size_t index = 0; index < m_size; index++) {
        m_pool.push_back(std::thread(
            &ThreadPool::__runtime,
            this,
            index
        ));
    }
}

void ThreadPool::stop(bool force)
{
    m_interrupt.store(true);
    for (size_t index = 0; index < m_size; index++) {
        (*m_mxcv[index].second).notify_all();
    }

    for (auto& thd : m_pool) {
        if (force) {
            pthread_cancel(thd.native_handle());
        }
        else {
            thd.join();
        }
    }
}

void ThreadPool::run_task(const Task& task)
{
    const size_t thread_id = m_last_used_thread_id.fetch_add(1) % m_size;
    {
        std::unique_lock<std::mutex> lock(*m_mxcv[thread_id].first);
        m_jobs[thread_id].push_back(task);
    }
    (*m_mxcv[thread_id].second).notify_one();
}

void ThreadPool::__runtime(const size_t thread_id)
{
    Task task;
    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(*m_mxcv[thread_id].first);
            (*m_mxcv[thread_id].second).wait(
                lock,
                [&]() {
                    return !m_jobs[thread_id].empty() || m_interrupt.load();
                }
            );

            if (m_interrupt.load()) {
                return;
            }

            task = m_jobs[thread_id].front();
            m_jobs[thread_id].pop_front();
        }
        task.execute();
    }
}

// -----------------------------------------------------------------------------
// USAGE
// -----------------------------------------------------------------------------
uint16_t shared_data[32];

void long_job(const uint16_t id)
{
    shared_data[id] = id;
    for(uint64_t i = 0; i < 100*1000*1000; i++) {}
}

int main()
{
    auto pool = ThreadPool(std::thread::hardware_concurrency());
    pool.start();

    for (uint16_t i = 0; i < 1024; i++)
    {
        auto job = std::function<void(const uint16_t)>(&long_job); 
        pool.run_task(Task(job, i));
    }

	std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "----" << std::endl;
    pool.stop();

    for (int i = 0; i < 1024; i++) {
	    std::cout << shared_data[i] << std::endl;
    }

    return 0;
}
