// this code is a sample target pseudocode that I'm planning to achieve in C++

// --------------------------------------------------
// Start of Draft 1
// --------------------------------------------------

// - Our unit of concurrency
// - The datatype that is returned by a coroutine
// - It is a group of data containing future response, execution pointer, etc
// - Resolved by scheduler by calling await() function and the awaitable as parameter
template <typename T = void>
class Awaitable
{
	public:
		Awaitable();
		T get_result() const;			// wait for response from callee
		something get_context() const;		// for restoration of caller?
		void function invoke_callee();		// and then set the future?
		function invoke_caller() const;		// yields and destorys awaitable?
	private:
		future<T> m_future;
		something m_context;
		function  m_callee;
		function  m_caller;
};


// currently the scheduler can be the threadpool also
template <typename T = void>
T await(Awaitable<T> const &awaitable, pinc::scheduler scheduler = pinc.get_event_loop())
{
	scheduler.add_task(awaitable);
	return awaitable.get_result();	// blocks this thread?
}


// where
void pinc::event_loop() {
	while(true)
		scheduler.get_task().invoke();
}


template <typename T = void>
async<T> coro_add(int a, int b)
{
	std::cout << "calculating response" << std::endl;
	await<>(pinc.sleep(5));		// yield control from this function to eventloop
	return a + b;			// finally return some value after
					// how to pass return value to awaitable?
}

template <typename T = void>
async<> root()
{
	std::cout << "asyncio app started" << std::endl;
	int response = await<int>(coro_add(5, 6));	// blocks
	std::cout << "received response" << response << std::endl;
	std::cout << "asyncio app finished" << std::endl;
}


int main() {
	return pinc.run(coro_add());	// blocks
}





// ----------------------------------------
// Start of Draft 2
// ----------------------------------------

