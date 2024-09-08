import asyncio


async def foo():
	for i in range(15):
		print(f"foo itr - {i}")
		await asyncio.sleep(1)
	return


async def bar():
	for i in range(10):
		print(f"bar itr - {i}")
		await asyncio.sleep(2)
	return


async def baz():
	for i in range(5):
		print(f"baz itr - {i}")
		await asyncio.sleep(3)
	return


async def main():
	await asyncio.gather(foo(), bar(), baz())


asyncio.run(main())
