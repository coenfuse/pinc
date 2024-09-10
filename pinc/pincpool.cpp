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

#define LABOR 5000000000

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


class ThreadPool
{
    public:
        ThreadPool(const uint16_t size = 1);
        void start();
        void stop(bool force = false);
        void run_task(const Task& task);

    private:
        void __runtime(const uint16_t& thread_id);

    private:
        uint16_t m_size;
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


// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------

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

ThreadPool::ThreadPool(const uint16_t size)
{
    m_size = size;
    m_last_used_thread_id.store(0);

    m_jobs.reserve(size);
    m_pool.reserve(size);
    m_mxcv.reserve(size);

    for (auto i = 0; i < size; i++)
    {
        m_jobs.push_back(std::deque<Task>());
        m_mxcv.push_back({
            std::make_unique<std::mutex>(),
            std::make_unique<std::condition_variable>()
        });
    }
}

void ThreadPool::start() {
    m_interrupt.store(false);
    for (auto index = 0; index < m_size; index++) {
        m_pool.push_back(std::thread(
            &ThreadPool::__runtime,
            this,
            index
        ));
    }
}

// evaluate methods to prevent memory leak during forceful shutdown, try to avoid
// orphaning the thread as it could cause malicous code to be executed into OS via
// PINC that may or may not be handleable by OS (trojan or virus)
void ThreadPool::stop(bool force) {
    m_interrupt.store(true);
}

void ThreadPool::run_task(const Task& task)
{
    uint16_t thread_id = m_last_used_thread_id.fetch_add(1) % m_size;
    {
        std::unique_lock<std::mutex> lock(*m_mxcv[thread_id].first);
        m_jobs[thread_id].push_back(task);
    }
    (*m_mxcv[thread_id].second).notify_one();
}

void ThreadPool::__runtime(const uint16_t& thread_id)
{
    Task task;
    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(*m_mxcv[thread_id].first);
            (*m_mxcv[thread_id].second).wait(
                lock,
                // std::chrono::milliseconds(500),
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
void long_job(const uint16_t id)
{
    for(uint64_t i = 0; i < LABOR; i++) {}
    std::cout << "job ended for task " << id << std::endl;
}

int main()
{
    auto pool = ThreadPool(std::thread::hardware_concurrency());
    pool.start();

    for (uint16_t i = 0; i < 32; i++)
    {
        auto job = std::function<void(const uint16_t)>(&long_job); 
        pool.run_task(Task(job, i));
    }

    std::this_thread::sleep_for(std::chrono::seconds(60));
    pool.stop();

    return 0;
}
