#include <coroutine>
#include <iostream>

std::coroutine_handle<> _HANDLE_;

void coro()
{
	std::cout << "IN - CORO_0" << std::endl;
	_HANDLE_ = std::coroutine_handle<>();
}

int main()
{
	std::cout << "IN - MAIN_0" << std::endl;

	_HANDLE_ = std::coroutine_handle<>();
	coro();

	std::cout << "IN - MAIN_1" << std::endl;
	_HANDLE_();

	return 0;
}
