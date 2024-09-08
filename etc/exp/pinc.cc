#include <chrono>
#include <iostream>
#include <setjmp.h>
#include <string>
#include <thread>

jmp_buf c1_cxt;
jmp_buf c2_cxt;

int coroutine1();
int coroutine2();

void println(std::string_view msg) {
    std::cout << msg << std::endl;
}

int coroutine1()
{
    int a = 5;

    println("CORO1 - started");
    println("CORO1 - working");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    println("CORO1 - saving context");
    if (setjmp(c1_cxt) == 0) {
        println("CORO1 - yielding to CORO2");
        coroutine2();
    } else {
        std::cout << "CORO1 - resuming context [ a = " << a << " ]" << std::endl;
    }

    println("CORO1 - working");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    println("CORO1 - saving context");
    if (setjmp(c1_cxt) == 0) {
        println("CORO1 - yielding to CORO2");
        longjmp(c2_cxt, 1);
    } else {
        std::cout << "CORO1 - resuming context [ a = " << a << " ]" << std::endl;
    }

    println("CORO1 - working");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    println("CORO1 - returns");
    return 0;
}

int coroutine2()
{
    int a = 10;

    println("CORO2 - started");
    println("CORO2 - working");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    println("CORO2 - saving context");
    if (setjmp(c2_cxt) == 0) {
        println("CORO2 - yielding to CORO1");
        longjmp(c1_cxt, 1);
    } else {
        std::cout << "CORO2 - resuming context [ a = " << a << " ]" << std::endl;
    }

    println("CORO2 - working");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    println("CORO2 - returns");
    return 0;
}

int main()
{
    coroutine1();
    return 0;
}