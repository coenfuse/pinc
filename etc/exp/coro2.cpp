#include <coroutine>
#include <iostream>
#include <string>


/*
class Awaitable 
{
	public:
		
	class promise_type;
	using handle_type = std::coroutine_handle<promise_type>;
		
	// plan on defining promise_type separately if necessary and use it as a
	// friend relationship with Awaitable. So that promise_type has access to
	// awaitable's ctor
	class promise_type
	{
		public:

		// Here we are basically specifiying what to return when a coroutine is
		// called, as you can see we are returning the awaitable class with a
		// coroutine handle embedded in it. The handle is a primitive that actually
		// contains local context, suspension, resume points etc.
		Awaitable get_return_object()
		{
			std::cout << "inside awaitable::promise_type::get_return_object" << std::endl;
			auto handle = handle_type::from_promise(*this);
			return Awaitable(handle);
		}

		// here we specify whether this coroutine should execute immediately upon
		// call or be suspended and wait for explicit request for start
		// with std::suspend_always, we tell it to suspend on start
		// with std::suspend_never, we tell it to execute on start and suspend later
		std::suspend_always initial_suspend()
		{ 
			return {}; 
		}

		// evaluated at co_return only if the co_return is returning void
		void return_void()
		{}

		// can't be defined if return_void() is defined, mutually exclusive
		// but is used when co_return is called with some value to return
		// it to caller or somewhere else, we can manage it here
		// void return_value()
		// {}

		// any unhandled exception inside the coroutine is captured and
		// sent here, currently we are silencing is and not doing anything
		// about it
		void unhandled_exception()
		{}
			
		// called before destroying of coroutine or line before co_return
		// or last line of coroutine. that is why is is noexcept since
		// we don't want any fight during destruction and let it pass
		// silently
		std::suspend_always final_suspend() noexcept
		{
			return {};
		}
			
		// this method basically resolves co_await
		std::suspend_always await_transform(Awaitable)
		{
			// you could write code here that adjusted the main
			// program's data structures to ensure the coroutine would
			// be resumed at the right time
			return {};
		}
	};

	private:

	handle_type m_handle;
	std::string m_name;

	// constructor intentionally made private so that only class's
	// internal methods and members (promise_type) can initialize it
	//
	// this is helpful is in future as any functionality changes in ctor,
	// the user outside of Awaitable doesn't have to change any code
	Awaitable(handle_type handle) : 
		m_handle(handle) 
		{
			m_name = "auto_coro";
			std::cout << "new auto_coro created" << std::endl;
		}

	public:
	Awaitable(){
		m_name = "manu_coro";
		std::cout << "new manu_coro created" << std::endl;
	}		
	
	void resume() {
		m_handle.resume();
	}
};

Awaitable coroutine() {
	std::cout << "in coroutine" << std::endl;
	co_await Awaitable();
	std::cout << "resumed coroutine and exiting now" << std::endl;
}

int main() {
	std::cout << "we're in main()" << std::endl;
	
	Awaitable instance = coroutine(); // initially suspended
	instance.resume();

	std::cout << "we're back in main()" << std::endl;
	instance.resume();

	std::cout << "we're back in main() and now terminating" << std::endl;
}
*/


/*
// conditional awaitable
if (condition())
	co_return
co_yield

// in the above, the conditional can be any boolean expression. It also sustains
// the timer functionaliy, if (current_ts_ns >= expiry_ts_ns): co_return; else co_yield

// whenever a coroutine yields, it must return the control back to its caller,
// here the executor or the scheduler. Since the executor will resume the line
// after it was suspended for coroutine it can either push it back to execution
// queue for later or destory it if the result is complete. Fucking beautiful.

int main():
	while (true):
		task = tasks.get()
		task.resume()
		if task.not_complete():
			tasks.push(task)
		else:
			task.destroy()
*/


namespace pinc
{
	template <char ID = '-'>
	class UserFacing;

	template <char ID = '-'>
	class promise_type;
}

template <char ID>
class pinc::UserFacing
{
	public:
	using promise_type = pinc::promise_type<ID>;
	using handle_type = std::coroutine_handle<promise_type>;
	friend promise_type;

	private:
	explicit UserFacing(handle_type handle){
		std::cout << "[" << ID << "] creating UserFacing Awaitable from handle provided by internal promise" << std::endl;
		m_handle = handle;
	}

	private:
	handle_type m_handle;

	public:

	~UserFacing() noexcept {
		std::cout << "[" << ID << "] coro complete, deleting handle" << std::endl;
		m_handle.destroy();
	}

	void resume(){
		std::cout << "[" << ID << "] trying to resume coro from caller" << std::endl;
		if (!m_handle.done()) {
			m_handle.resume();
		}
	}
};

template <char ID>
class pinc::promise_type
{
	public:

	pinc::UserFacing<ID> get_return_object()
	{
		std::cout << "[" << ID << "] creating a handle for coro and returning to caller wrapped inside an awaitable" << std::endl;
		auto m_handle = pinc::UserFacing<ID>::handle_type::from_promise(*this);
		return UserFacing<ID>(m_handle);
	}

	std::suspend_always initial_suspend() {
		std::cout << "[" << ID << "] performing initial suspend" << std::endl;
		return {};
	}

	void return_void() {
		std::cout << "[" << ID << "] handling void return" << std::endl;
	}

	void unhandled_exception() noexcept
	{}

	std::suspend_always final_suspend() noexcept {
		std::cout << "[" << ID << "] performing final suspend" << std::endl;
		// m_handle.destroy();
		return {};
	}

	std::suspend_always await_transform(pinc::UserFacing<ID> awaitable) {
		std::cout << "[" << ID << "] suspending coro with awaitable UserFacing" << std::endl;
		awaitable.resume();
		return {};
	}

	std::suspend_always yield_value(int value) {
		return {};
	}

	// private:
	// typename pinc::UserFacing<ID>::handle_type m_handle;
};


pinc::UserFacing<'f'> baz() {
	std::cout << "in baz" << std::endl;
	co_return;
}


pinc::UserFacing<'f'> foo() {
	std::cout << "in coro" << std::endl;
	co_await baz();
	
	std::cout << "back in coro" << std::endl;
	co_return;
}


int main() 
{
	pinc::UserFacing<'f'> instance = foo();

	std::cout << "in main" << std::endl;
	instance.resume();

	std::cout << "back in main" << std::endl;
	instance.resume();

	std::cout << "again back in main" << std::endl;
}