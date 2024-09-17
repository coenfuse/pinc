#include <concepts>
#include <coroutine>
#include <exception>
#include <iostream>


struct ReturnObject
{
	struct promise_type
	{
		ReturnObject get_return_object() {
			return {};
		}

		std::suspend_never initial_suspend() {
			return {};
		}

		std::suspend_never final_suspend() noexcept {
			return {};
		}

		void unhandled_exception()
		{}
	};
};


struct ReturnObject2
{
	struct promise_type
	{
		ReturnObject2 get_return_object() {
			return {
				// Uses C++20 designated initializer syntax
				.h_  = std::coroutine_handle<promise_type>::from_promise(*this)
			};
		}

		std::suspend_never initial_suspend() {
			return {};
		}

		std::suspend_never final_suspend() noexcept {
			return {};
		}

		void unhandled_exception() {
		}
	};

	std::coroutine_handle<promise_type> h_;
	operator std::coroutine_handle<promise_type>() const { return h_; }
	// A coroutine_handle<promise_type> converts to coroutine_handle<>
	operator std::coroutine_handle<>() const {
		return h_;
	}
};


struct Awaiter
{
	std::coroutine_handle<> *hp_;

	constexpr bool await_ready() const noexcept {
		return false;
	}

	void await_suspend(std::coroutine_handle<> h) {
		*hp_ = h;
	}

	constexpr void await_resume() const noexcept
	{}
};


void counter(std::coroutine_handle<> *continuation_out)
{
	std::cout << "entered counter" << std::endl;
	Awaiter a{continuation_out};
	for (unsigned i = 0; ; ++i) {
		co_await a;
		std::cout << "counter: " << i << std::endl;
	}
}


int main()
{
	std::coroutine_handle<> h;
	counter(&h);

	for (int i = 0; i < 3; ++i) 
	{
		std::cout << "In main1 function\n";
		h();
	}

	h.destroy();
	return 0;
}
