#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <thread>

// uint8_t is a typedef for unsigned char in C++
// due to this, ostream class (used for cout) prints the character having ascii
// numeric value. And here for 0, it prints nothing. Thus, use uint16_t

struct Task
{
    void (*fptr)(uint16_t param);
    uint16_t param;
};


struct ThreadPool
{
    std::mutex m_mtx;
    bool m_interrupted = false;
    std::condition_variable m_cv;

    uint16_t m_size = 1;
    uint16_t m_last_used_thread = 0;
    std::map<uint16_t, std::thread> m_pool;
    std::map<uint16_t, std::queue<Task>> m_jobs;
    std::map<uint16_t, std::pair<std::condition_variable, std::mutex>> m_ctrl;
};


void initialize_pool(ThreadPool& pool, const uint16_t& size) 
{
    pool.m_size = size;
    for (uint16_t i = 0; i < size; i++) {
        pool.m_jobs[i] = std::queue<Task>();
    }
}


void thread_pool_runtime(ThreadPool& pool, uint16_t thread_id)
{
    Task task;
    while (!pool.m_interrupted)
    {
        {
            const std::lock_guard<std::mutex> lock(pool.m_mtx);
            if (pool.m_jobs[thread_id].empty())
                continue;
            task = pool.m_jobs[thread_id].front();
            pool.m_jobs[thread_id].pop();
        }

        try {
            task.fptr(task.param);
        }
        catch (...)
        {}
    }
}


void start_pool(ThreadPool& pool)
{
    pool.m_interrupted = false;
    for (uint16_t index = 0; index < pool.m_size; index++) {
        pool.m_pool[index] = std::thread(&thread_pool_runtime, std::ref(pool), index);
    }
}


void stop_pool(ThreadPool& pool)
{
    pool.m_interrupted = true;
    for (auto& [index, thread] : pool.m_pool) {
        thread.join();
    }
}


const uint16_t compute_worker_thread(ThreadPool& pool) 
{
    if (pool.m_last_used_thread == pool.m_size - 1) {
        return 0;
    }
    else {
        return pool.m_last_used_thread + 1;
    }
}


const uint16_t add_task(ThreadPool& pool, const Task& task)
{
    const uint16_t worker_thread_id = compute_worker_thread(pool);
    {
        const std::lock_guard<std::mutex> lock(pool.m_mtx);
        pool.m_jobs[worker_thread_id].push(task);
    }
    pool.m_last_used_thread = worker_thread_id;
    return pool.m_last_used_thread;
}


void long_job(const uint16_t duration)
{
    std::cout << "JOB [" << duration << "] - started" << std::endl;
    for (uint16_t age = 0; age < duration; age++) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "JOB [" << duration << "] - finished" << std::endl;
}

int main()
{
    ThreadPool pool = ThreadPool();
    initialize_pool(pool, std::thread::hardware_concurrency());
    start_pool(pool);

    for (uint16_t i = 0; i < 64; i++) {
        Task t = Task();
        t.fptr = &long_job;
        t.param = 1 + (i % 2);
        add_task(pool, t);
    }

    // let pool run tasks for a moment
    std::this_thread::sleep_for(std::chrono::seconds(20));
    stop_pool(pool);
    return 0;
}