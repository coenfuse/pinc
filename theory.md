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