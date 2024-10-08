Let's understand the following synchronous code
```c
int bar():
    return 4;

int foo():
    int a = 1;
    int b = 2;
    return a + b + bar();

void main():
    int val = foo();
    return;
```

Assume the stack pointer (RSP) currently at 0.
When this program is not loaded onto memory, the stack looks like this
|                | - 00 <-- RSP

After loading, the stack frame shall look like this
|  START: main   | - 00 <-- RSP

now the executor will evaluate "main()" and upon executing its first line, it
allocates memory as follows

| int main:value | - 103
| int main:value | - 102
| int main:value | - 101
| int main:value | - 100

where main:value means a variable of named 'value' belonging to function 'main'
having type 'int'. In this system int occupies 4 bytes. Thus int main:value occupies
bytes from 100 to 103 on memory. Furthermore, for simplicity sakes 'main' is a unique
function signature using with executor can find the function pointer. Currently
the function pointer is pointing to stack pointer 0.

Now that the allocation is done, the RHS of the statement is evaluated that happens
to be a function call. So simply, the executor will CALL foo() from main(). To do
so, it must suspend the execution of main and jump to function pointer of foo().

Since none exists on stack, foo is added on top of execution stack resulting,
|  START: foo    | - 00 <-- RSP
|  START: main   | - 00

Now that is added on top of the execution stack. The system was previously executing
main but has now to suspend it since a line it its execution requires evaluation of
foo. If we interpolate both memory and execution stack we get the following,

| START: foo    | - 05 <-- RSP
| int main:val  | - 04
| int main:val  | - 03
| int main:val  | - 02
| int main:val  | - 01
| START: main   | - 00

So function main() starts at address 0 and current suspended at 4. It is now
blocked by foo that starts from 5 which upon return shall provide a value that
initializes addresses 1 to 4. Since all the variables of main are declared at
compile time, they are in frame.

We'll keep this interpolated stack for understanding from now on.
When we execute foo's first line, we have an updated stack as

| int foo:a     | - 09 - 00 00 00 01 <-- RSP
| int foo:a     | - 08 - 00 00 00 00
| int foo:a     | - 07 - 00 00 00 00
| int foo:a     | - 06 - 00 00 00 00
| START: foo    | - 05 - 00 00 01 01
| int main:val  | - 04 - * GARBAGE *
| int main:val  | - 03 - * GARBAGE *
| int main:val  | - 02 - * GARBAGE *
| int main:val  | - 01 - * GARBAGE *
| START: main   | - 00 - 00 00 00 00

For simplicity sakes, the start value of function points is same as their stack
pointer value. Now after allocating and initializing foo:b, we have

| int foo:b     | - 13 - 00 00 00 10 <-- RSP
| int foo:b     | - 12 - 00 00 00 00
| int foo:b     | - 11 - 00 00 00 00
| int foo:b     | - 10 - 00 00 00 00
| int foo:a     | - 09 - 00 00 00 01
| int foo:a     | - 08 - 00 00 00 00
| int foo:a     | - 07 - 00 00 00 00
| int foo:a     | - 06 - 00 00 00 00
| START: foo    | - 05 - 00 00 01 01
| int main:val  | - 04 - * GARBAGE *
| int main:val  | - 03 - * GARBAGE *
| int main:val  | - 02 - * GARBAGE *
| int main:val  | - 01 - * GARBAGE *
| START: main   | - 00 - 00 00 00 00

Now we evaluate foo:return,
this must mean that the current function's stack frame shall be destroyed the RSP
should point to line just before/after it was called by the caller. Then RSP should
be executing the caller from where it was suspended.

So, the stack frame after evaluation must look like, here foo:ret is a temporary
variable created by foo to momentarily store the evaluation result before passing
it to the caller. Furthermore, since foo's function signature told us that it will
return a value of type int. 4 bytes are reserved by foo:ret

| int foo:ret   | - 17 - * GARBAGE * <-- RSP
| int foo:ret   | - 16 - * GARBAGE *
| int foo:ret   | - 15 - * GARBAGE *
| int foo:ret   | - 14 - * GARBAGE *
| int foo:b     | - 13 - 00 00 00 10
| int foo:b     | - 12 - 00 00 00 00
| int foo:b     | - 11 - 00 00 00 00
| int foo:b     | - 10 - 00 00 00 00
| int foo:a     | - 09 - 00 00 00 01
| int foo:a     | - 08 - 00 00 00 00
| int foo:a     | - 07 - 00 00 00 00
| int foo:a     | - 06 - 00 00 00 00
| START: foo    | - 05 - 00 00 01 01
| int main:val  | - 04 - * GARBAGE *
| int main:val  | - 03 - * GARBAGE *
| int main:val  | - 02 - * GARBAGE *
| int main:val  | - 01 - * GARBAGE *
| START: main   | - 00 - 00 00 00 00

However, the executor isn't able to evaluate foo:ret since it invokes another call
to bar(). So the stack frame looks like (moment after executing bar()),

| END: bar      | - 23 - 00 01 01 11 <-- RSP
| int bar:ret   | - 22 - 00 00 01 00
| int bar:ret   | - 21 - 00 00 00 00
| int bar:ret   | - 20 - 00 00 00 00
| int bar:ret   | - 19 - 00 00 00 00
| START: bar    | - 18 - 00 01 00 10
| int foo:ret   | - 17 - * GARBAGE *
| int foo:ret   | - 16 - * GARBAGE *
| int foo:ret   | - 15 - * GARBAGE *
| int foo:ret   | - 14 - * GARBAGE *
| int foo:b     | - 13 - 00 00 00 10
| int foo:b     | - 12 - 00 00 00 00
| int foo:b     | - 11 - 00 00 00 00
| int foo:b     | - 10 - 00 00 00 00
| int foo:a     | - 09 - 00 00 00 01
| int foo:a     | - 08 - 00 00 00 00
| int foo:a     | - 07 - 00 00 00 00
| int foo:a     | - 06 - 00 00 00 00
| START: foo    | - 05 - 00 00 01 01
| int main:val  | - 04 - * GARBAGE *
| int main:val  | - 03 - * GARBAGE *
| int main:val  | - 02 - * GARBAGE *
| int main:val  | - 01 - * GARBAGE *
| START: main   | - 00 - 00 00 00 00

The executor must know that when "END: func" is called, it must move the RSP from
current position to one place below "START: func". Furthermore, if func returns
something, it must capture it and pass it to the now resumed function. And also
destory any local variables by deallocating the stack.

Note that, had it been the system's limiation to only have depth of 20 in stack
frame, we'd have seen stack overflow.

Now after deallocating FUNC:bar and returning its value to it's caller, foo. We
have ourselves the stack frame as,

| END: foo      | - 18 - 00 01 00 10 <-- RSP
| int foo:ret   | - 17 - 00 00 01 11
| int foo:ret   | - 16 - 00 00 00 00
| int foo:ret   | - 15 - 00 00 00 00
| int foo:ret   | - 14 - 00 00 00 00
| int foo:b     | - 13 - 00 00 00 10
| int foo:b     | - 12 - 00 00 00 00
| int foo:b     | - 11 - 00 00 00 00
| int foo:b     | - 10 - 00 00 00 00
| int foo:a     | - 09 - 00 00 00 01
| int foo:a     | - 08 - 00 00 00 00
| int foo:a     | - 07 - 00 00 00 00
| int foo:a     | - 06 - 00 00 00 00
| START: foo    | - 05 - 00 00 01 01
| int main:val  | - 04 - * GARBAGE *
| int main:val  | - 03 - * GARBAGE *
| int main:val  | - 02 - * GARBAGE *
| int main:val  | - 01 - * GARBAGE *
| START: main   | - 00 - 00 00 00 00

Kindly note that foo:ret value has now been evaluated to 7 by evaluatnig the
expression and is now stored temporarily in foo:ret. Now since foo can return, we
can deallocate the whole foo: frame from stack and we have ourselves frame as

| int main:val  | - 04 - 00 00 01 11 <-- RSP
| int main:val  | - 03 - 00 00 00 00
| int main:val  | - 02 - 00 00 00 00
| int main:val  | - 01 - 00 00 00 00
| START: main   | - 00 - 00 00 00 00

The garbage value of main:val is now overwritten with value stored in foo:ret.
Now we finally reach return of main and the executor executes END: main

Which instructs it to move the RSP to a point preceding to start of main.
Which in turn gives an empty stack frame as

|               | - 00

Exactly what we started with. This means completion of a program.




import asyncio
import time

def sync_ops():
    time.sleep(2)
    return

async def qux():
    print("qux() - start")
    print("qux() - suspend on to_thread(sync_ops)")
    await asyncio.to_thread(sync_ops)
    print("qux() - resume")
    print("qux() - exit")

async def baz():
    print("baz() - start")
    for _ in range(1000 * 1000 * 500):
        pass
    print("baz() - exit")
    return

async def bar():
    print("bar() - start")
    print("bar() - suspend on gather()")
    await asyncio.gather(*[
        qux(),
        baz(),
    ])
    print("bar() - resume")
    print("bar() - exit")

async def ted():
    print("ted() - start")
    print("ted() - suspend on sleep(1)")
    await asyncio.sleep(1)
    print("ted() - resume")
    print("ted() - exit")
    return

async def foo():
    print("foo() - start")
    print("foo() - suspend on ted()")
    await ted()
    print("foo() - resume")

    print("foo() - suspend on gather()")
    await asyncio.gather(*[
        bar(),
        baz(),
        qux(),
        ted()
    ])
    print("foo() - resume")
    print("foo() - exit")

def main():
    asyncio.run(foo())

main()


# CONSOLE OUTPUT [EXPECTED]
# ------------------------------------------------------------------------------
# foo() - start
# foo() - suspend on ted()
# ted() - start
# ted() - suspend on sleep(1)
# ted() - resume
# ted() - exit
# foo() - resume
# foo() - suspend on gather()
# bar() - start
# bar() - suspend on gather()
# baz() - start
# baz() - exit
# qux() - start
# qux() - suspend on to_thread()
# ted() - start
# ted() - suspend on sleep()
# qux() - start
# qux() - suspend on to_thread()
# baz() - start
# baz() - exit
# qux() - resume
# qux() - exit
# qux() - resume
# qux() - exit
# bar() - resume
# bar() - exit
# ted() - resume
# ted() - exit
# foo() - resume
# foo() - exit


# EXECUTION LOG [UNSURE]
# ------------------------------------------------------------------------------
# let's have,
# CS = abbr. for 'call-stack' containing active function calls
# TQ = abbr. for 'task-queue' containing general coroutines
# CQ = abbr. for 'conditional-queue' containing coroutines waiting on a condition
# CM = abbr. for 'context memory' containing local variables
#
#
# 00.000 > push main() in CS
# CS - [main]
# TQ - []
# CQ - []
# CM - []
#
# 00.001 > push asyncio.run() in CS
# CS - [main > asyncio.run]
# TQ - []
# CQ - []
# CM - []
#
# 00.002 > push foo() in TQ
# CS - [main > asyncio.run]
# TQ - [foo]
# CQ - []
# CM - []
#
# 00.003 > pop foo() from TQ
# CS - [main > asyncio.run > TQ.get]
# TQ - []
# CQ - []
# CM - [foo]
#
# 00.004 > push foo()::resume() in CS
# CS - [main > asyncio.run > foo.resume]
# TQ - []
# CQ - []
# CM - [foo]
#
# 00.005 > push await() in CS
# CS - [main > asyncio.run > foo.resume > await]
# TQ - []
# CQ - []
# CM - [foo]
# 
# 00.006 > push ted() in TQ
# CS - [main > asyncio.run > foo.resume > await > TQ.push]
# TQ - []
# CQ - []
# CM - [foo]
#
# 00.007 > pop TQ.push() from CS
# CS - [main > asyncio.run > foo.resume > await]
# TQ - [ted]
# CQ - []
# CM - [foo]
#
# 00.008 > suspend foo() into TQ
# CS - [main > asyncio.run > foo.resume > await]
# TQ - [foo > ted]
# CQ - []
# CM - []
# 
# 00.009 > pop await() from CS
# CS - [main > asyncio.run > foo.resume]
# TQ - [foo > ted]
# CQ - []
# CM - []
#
# 00.009 > pop foo()::resume() from CS since it returned on foo's suspension
# CS - [main > asyncio.run]
# TQ - [foo > ted]
# CQ - []
# CM - []
#
# 00.010 > pop ted() from TQ
# CS - [main > asyncio.run > TQ.get]
# TQ - [foo]
# CQ - []
# CM - [ted]
# 
# 00.011 > push ted()::resume() in CS
# CS - [main > asyncio.run > ted.resume]
# TQ - [foo]
# CQ - []
# CM - [ted]
#
# 00.012 > push await() in CS
# CS - [main > asyncio.run > ted.resune > await]
# TQ - [foo]
# CQ - []
# CM - [ted]
#
# 00.013 > push sleep() in CQ
# CS - [main > asyncio.run > ted.resune > await > CQ.push]
# TQ - [foo]
# CQ - [sleep]
# CM - [ted]
#
# 00.014 > pop CQ.push from CS
# CS - [main > asyncio.run > ted.resume > await]
# TQ - [foo]
# CQ - [sleep]
# CM - [ted]
#
# 00.015 > suspend ted() in TQ
# CS - [main > asyncio.run > ted.resume > await]
# TQ - [ted > foo]
# CQ - [sleep]
# CM - []
#
# 00.016 > pop await() from CS
# CS - [main > asyncio.run > ted.resume]
# TQ - [ted > foo]
# CQ - [sleep]
# CM - []
#
# 00.017 > pop ted()::resume() from CS since it returned on ted's suspension
# CS - [main > asyncio.run]
# TQ - [ted > foo]
# CQ - [sleep]
# CM - []
#
# 00.018 > pop sleep() from CQ
# CS - [main > asyncio.run]
# TQ - [ted > foo]
# CQ - []
# CM - [sleep]
# 
# 00.019 > push sleep()::is_done() in CS -- basically busy wait :(
# CS - [main > asyncio.run > sleep.is_done]
# TQ - [ted > foo]
# CQ - []
# CM - [sleep]
#
# 00.020 > pop sleep()::is_done() from CS (returns false)
# CS - [main > asyncio.run]
# TQ - [ted > foo]
# CQ - []
# CM - [sleep]
#
# 00.021 > push sleep()::set_eval_cycle() in CS -- this tells event loop that it has evaluated this coroutine in this cycle and needs to ignore it until next iteration. Else, it will freeze on infinite loop on CQ starving TQ. The eval flag can be a boolean and scheduler iteration would be either 0 or 1. So any coro set with eval_cycle = 1 will not be run again until the eval_cycle != 1
# CS - [main > asyncio.run > sleep.set_eval_cycle]
# TQ - [ted > foo]
# CQ - []
# CM - [sleep]
#
# 00.022 > pop sleep()::set_eval_cycle() in CS
# CS - [main > asyncio.run]
# TQ - [ted > foo]
# CQ - []
# CM - [sleep]
#
# 00.023 > push sleep() in CQ
# CS - [main > asyncio.run > CQ.push]
# TQ - [ted > foo]
# CQ - []
# CM - [sleep]
#
# 00.024 > pop CQ.push from CS
# CS - [main > asyncio.run]
# TQ - [ted > foo]
# CQ - [sleep]
# CM - []
#
# 00.025 > pop foo() into CM
# CS - [main > asyncio.run > TQ.pop]
# TQ - [ted > foo]
# CQ - [sleep]
# CM - []
#
# 00.030 > pop TQ.pop from CS
# CS - [main > asyncio.run]
# TQ - [ted]
# CQ - [sleep]
# CM - [foo]
#
# 00.030 > push foo().is_resumable() in CS
# CS - [main > asyncio.run > foo.is_resumable]
# TQ - [ted]
# CQ - [sleep]
# CM - [foo]
#
# 00.031 > pop foo().is_resumable() from CS [returned false]
# CS - [main > asyncio.run]
# TQ - [ted]
# CQ - [sleep]
# CM - [foo]
#
# 00.032 > push foo() in TQ
# CS - [main > asyncio.run]
# TQ - [foo > ted]
# CQ - [sleep]
# CM - []
#
# 00.033 > pop ted() into CM
# CS - [main > asyncio.run > TQ.pop]
# TQ - [foo]
# CQ - [sleep]
# CM - [ted]
#
# 00.034 > pop TQ.pop from CS
# CS - [main > asyncio.run]
# TQ - [foo]
# CQ - [sleep]
# CM - [ted]
#
# 00.035 > push ted().is_resumable() in CS
# CS - [main > asyncio.run > ted.is_resumable]
# TQ - [foo]
# CQ - [sleep]
# CM - [ted]
# 
# 00.036 - pop ted().is_resumable() from CS [returned false]
# CS - [main > asyncio.run]
# TQ - [foo]
# CQ - [sleep]
# CM - [ted]
#
# 00.037 - push ted() in TQ
# CS - [main > asyncio.run]
# TQ - [ted > foo]
# CQ - [sleep]
# CM - []
# 
# -- repeat cycle from 00.018 to 00.037
# -- until you are at a moment of completion of sleep
# -- i.e., 01.013 since sleep was created at 00.013
# 
# 01.013 - push sleep()::is_done() in CS
# CS - [main > asyncio.run > sleep.is_done]
# TQ - [ted > foo]
# CQ - []
# CM - [sleep]
#
# 01.014 - pop sleep()::is_done() from CS (returns true)
# CS - [main > asyncio.run]
# TQ - [ted > foo]
# CQ - []
# CM - [sleep]
#
# 01.015 - push sleep()::execute_cb() in CS
# CS - [main > asyncio.run > sleep.execute_cb]
# TQ - [ted > foo]
# CQ - []
# CM - [sleep]
#
# 01.016 - push ted().resume() since it was registered as callback in sleep()
# CS - [main > asyncio.run > sleep.execute_cb > ted.resume]
# TQ - [ted > foo]
# CQ - []
# CM - [sleep]
#
# -- so sleep() blocked TED for 1.003 seconds (kinda bad)
# 
# 01.017 - pop ted.resume() from CS since it returned after execution
# CS - [main > asyncio.run > sleep.execute_cb]
# TQ - [ted > foo]
# CQ - []
# CM - [sleep]
#
# 01.018 - pop sleep()::execute_cb() from CS
# CS - [main > asyncio.run]
# TQ - [ted > foo]
# CQ - []
# CM - [sleep]
#
# 01.019 - push sleep()::destroy() in CS to release memory occupied by this coro
# CS - [main > asyncio.run > sleep.destroy]
# TQ - [ted > foo]
# CQ - []
# CM - [sleep]
#
# 01.020 - pop sleep()::destroy()
# CS - [main > asyncio.run]
# TQ - [ted > foo]
# CQ - []
# CM - [sleep]
#
# 01.021 - pop sleep() from CM
# CS - [main  > asyncio.run]
# TQ - [ted > foo]
# CQ - []
# CM - []
# 
# 01.022 - pop foo() in CM
# CS - [main > asyncio.run]
# TQ - [ted]
# CQ - []
# CM - [foo]
#
# 01.023 - push foo().is_resumable() in CS
# CS - [main > asyncio.run > foo.is_resumable]
# TQ - [ted]
# CQ - []
# CM - [foo]
#
# 01.024 - pop foo().is_resumable() in CS [return false] & pop foo() back into TQ
# CS - [main > asyncio.run]
# TQ - [foo > ted]
# CQ - []
# CM - []
#
# 01.026 - pop ted() into CM
# CS - [main > asyncio.run]
# TQ - [foo]
# CQ - []
# CM - [ted]
#
# 01.027 - push ted()::is_resumable() into CS
# CS - [main > asyncio.run > ted.is_resumable]
# TQ - [foo]
# CQ - []
# CM - [ted]
#
# 01.028 - pop ted()::is_resumable() from CS (returned false)
# CS - [main > asyncio.run]
# TQ - [foo]
# CQ - []
# CM - [ted]
#
# 01.029 - push ted()::is_done() in CS
# CS - [main > asyncio.run > ted.is_done]
# TQ - [foo]
# CQ - []
# CM - [ted]
#
# 01.030 - pop ted()::is_done() from CS (returns true)
# CS - [main > asyncio.run]
# TQ - [foo]
# CQ - []
# CM - [ted]
#
# 01.031 - push ted()::execute_cb() in CS
# CS - [main > asyncio.run > ted.execute_cb]
# TQ - [foo]
# CQ - []
# CM - [ted]
#
# 01.032 - push foo()::resume() in CS since it was registered CB in ted()
# CS - [main > asyncio.run > ted.execute_cb > foo.resume]
# TQ - [foo]
# CQ - []
# CM - [ted]
#
# -- CS and scheduler frozen until foo.resume returns or suspends
# 
# 01.033 - push await into CS as foo.resume hits suspension point
# CS - [main > asyncio.run > ted.execute_cb > foo.resume > await]
# TQ - [foo]
# CQ - []
# CM - [ted]
#
# 01.034 - push gather into TQ
# CS - [main > asyncio.run > ted.execute_cb > foo.resume > await > TQ.push]
# TQ - [foo.gather > foo]
# CQ - []
# CM - [ted]
#
# 01.035 - pop TQ.push, await, foo.resume and ted.execute_cb as they are unblocked
# CS - [main > asyncio.run]
# TQ - [foo.gather > foo]
# CQ - []
# CM - [ted]
#
# 01.036 - push ted()::destroy() into CS
# CS - [main > asyncio.run > ted.destory]
# TQ - [foo.gather > foo]
# CQ - []
# CM - [ted]
#
# 01.037 - pop ted()::destory() from CS, delete ted() from CM
# CS - [main > asyncio.run]
# TQ - [foo.gather > foo]
# CQ - []
# CM - []
#
# -- perform gather, populate TQ and repeat as above
# -- todo