#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>

void println(const char* msg){
	printf("%s\n", msg);
	fflush(stdout);			// flush the std output to unblock sleep() or other calls in case \n is not provided
}

jmp_buf c1_cxt;
jmp_buf c2_cxt;

int coro1();
int coro2();

int coro1()
{
	int a = 5;

	println("coro1 started");
	sleep(1);							// simulate some work

	println("coro1 saving its context");
	printf("context int a = %d\n", a);
	if (setjmp(c1_cxt) == 0){
		println("coro1 yielding control to coro2");
		coro2();
	}
	else{
		println("coro1 resuming");
		printf("context int a = %d\n", a);
		sleep(1);
	}

	println("coro1 saving its context");
	printf("context int a = %d\n", a);
	if(setjmp(c1_cxt) == 0){
		println("coro1 yielding control to coro2");
		longjmp(c2_cxt, 1);
	}
	else {
		println("coro1 resuming");
		printf("context int a = %d\n", a);
		sleep(1);						// simulate some work
	}

	println("coro1 terminating");
	return 0;
}

int coro2()
{
	println("coro2 started");
	// int a = 6;

	sleep(2);							// simulate some work

	println("coro2 saving its context");
	if (setjmp(c2_cxt) == 0)
	{
		println("coro2 yielding control back to coro1");
		longjmp(c1_cxt, 1);
	}
	else {
		println("coro2 resuming");
		sleep(1);						// simulate some work
	}

	println("coro2 terminating");
	longjmp(c1_cxt, 1);  // Safely jump back to coro1 to avoid continuing after longjmp and cause segfault in coro1 local vars
	// free(c2_cxt);	     // Need help in avoiding memory leak here, this raises abortion. Can't assign to NULL due to non-modifiable lvalue
	setjmp(c2_cxt);
	return 0;
}

int main()
{
	coro1();
	println("");

	// longjmp(c2_cxt, 1);
	return 0;
}
