#include <iostream>

struct Foo
{
	int val;
	int man;
};

int main()
{
	Foo a{45, 56};
	std::cout << a.val << ", " << a.man << std::endl;
	return 0;
}
