import uvloop
import asyncio

data = ""
interrupt = asyncio.Event()

async def cooperative_printer():
	global data
	while not interrupt.is_set():
		if len(data) > 0:
			print(data)
		await asyncio.sleep(0.5)

async def read_input():
	global data
	loop = asyncio.get_running_loop()
	while not interrupt.is_set():
		try:
			new_data = await loop.run_in_executor(None, input, "\nEnter new data - ")
			data = new_data
		except KeyboardInterrupt:
			interrupt.set()

async def main():
	tasks = [cooperative_printer() for _ in range(1000000000)]
	input_tasks = read_input()
	await asyncio.gather(*tasks, input_tasks)

uvloop.run(main())
