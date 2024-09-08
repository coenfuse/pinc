from concurrent.futures import Future
import queue
import random
import threading
import time


class Task:

	def __init__(self, job, params):
		self.__params = params
		self.__defptr = job
		self.__result = Future()

	def run(self):
		self.__result.set_result(self.__defptr(self.__params))

	def get_result(self):
		self.__result.result()


class ThreadPool:

	def __init__(self, size):
		self.__task_queues = {}
		self.__thread_objs = {}
		self.__interrupt   = threading.Event()

		for index in range(size):
			self.__task_queues[index] = queue.Queue()
			self.__thread_objs[index] = threading.Thread(
				target = self.__runtime,
				args   = [index]
			)

		self.__last_used_thread_id = 0


	def add_task(self, task: Task):
		# self.__task_queues[random.randint(0, len(self.__thread_objs) - 1)].put(task)
		worker_thread_id = self.__get_candidate_worker_id()
		self.__task_queues[worker_thread_id].put(task)
		self.__last_used_thread_id = worker_thread_id


	def start(self):
		self.__interrupt.clear()
		for thread in self.__thread_objs.values():
			thread.start()

	
	def stop(self):
		self.__interrupt.set()
		for thread in self.__thread_objs.values():
			thread.join()


	def __runtime(self, task_queue_id: int):
		# print(f"running thread - {task_queue_id}")
		while not self.__interrupt.is_set():
			try:
				task = self.__task_queues[task_queue_id].get(timeout = 0.1)
				task.run()
			except queue.Empty:
				pass


	def __get_candidate_worker_id(self):
		if self.__last_used_thread_id == len(self.__thread_objs) - 1:
			return 0
		else:
			return self.__last_used_thread_id + 1
		


def main():

	def long_job(dur):
		print(f"JOB [{dur}] - started")
		start = time.time()
		for _ in range(0, dur):
			time.sleep(1)
		print(f"JOB [{dur}] - ended with delta {time.time() - start}")

	
	deadpool = ThreadPool(1024)
	deadpool.start()

	for _ in range(1024):
		deadpool.add_task(Task(long_job, 1))

main()