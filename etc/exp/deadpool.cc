#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <queue>
#include <thread>
#include <vector>

class Task
{
	public:
	Task(void (*)(int), int);
	void execute() const;
	// awaitable get_result();

	private:
	void (*m_fptr)(int);
	int m_param;
};


class ThreadPool
{
	public:
	ThreadPool(const uint16_t&);
	uint16_t add_task(const Task&);
	void start();
	void stop();

	private:
	void __runtime(const uint16_t&);
	const uint16_t __get_candidate_worker_id() const;

	private:
	std::map<uint16_t, std::queue<Task>> m_task_queues;
	std::map<uint16_t, std::thread> m_threads;
	uint16_t m_last_utilized_thread = 0;
	bool m_interrupt = false;
};

// API definitions
Task::Task(void (*fptr)(int), int param)
{
	m_fptr 	= fptr;
	m_param = param;
}

void Task::execute() const {
	m_fptr(m_param);
}

ThreadPool::ThreadPool(const uint16_t& pool_size) {
	for (uint16_t index = 0; index < pool_size; index++) {
        m_task_queues[index] = std::queue<Task>();
    }
}

uint16_t ThreadPool::add_task(const Task& task) 
{
	const uint16_t worker_thread_id = __get_candidate_worker_id();
	m_task_queues[worker_thread_id].push(task);
	m_last_utilized_thread = worker_thread_id;
	return worker_thread_id;
}

void ThreadPool::start()
{
	m_interrupt = false;
	for(uint16_t index = 0; index < m_task_queues.size(); index++) 
	{
        std::thread thread(&ThreadPool::__runtime, this, index);
        m_threads.insert(std::make_pair(index, std::move(thread)));
    }
}

void ThreadPool::stop()
{
	m_interrupt = true;
    for (auto& [id, thread] : m_threads) {
        thread.join();
    }
}

void ThreadPool::__runtime(const uint16_t& queue_id)
{
	while (!m_interrupt)
	{
		try
		{
			const Task& task = m_task_queues[queue_id].front();
			task.execute();
			m_task_queues[queue_id].pop();
		}
		catch (...)
		{}
	}
}

const uint16_t ThreadPool::__get_candidate_worker_id() const
{
	if (m_last_utilized_thread == m_task_queues.size() - 1) {
		return 0;
	}
	else {
		return m_last_utilized_thread + 1;
	}
}


// Application Logic
void long_job(int duration) 
{
	std::cout << "JOB [" << duration << "] - started" << std::endl;
	uint start = 0;

	for(int i = 0; i < duration; i++) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	std::cout << "JOB [" << duration << "] - ended with delta " << 0 - start << std::endl;
}

int main()
{
	ThreadPool deadpool = ThreadPool(1);
	deadpool.start();

	for (int i = 0; i < 16; i++) {
		deadpool.add_task(Task(long_job, 1));
	}

	std::this_thread::sleep_for(std::chrono::seconds(20));
	deadpool.stop();

	return 0;
}