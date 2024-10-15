# Fibers
The "std/fiber" library provides support for the Fiber (coroutine) class.
## Creation
Create a fiber with the following statement:
```
ref "std/fiber"

fiber = Fiber(func(arg) {
    do_some_thing()
    yield 1
    do_some_thing()
    yield 2
    do_some_thing()
    return 1
}, null, "arg")
```
The parameters for calling a Fiber are:

+ func: The thread execution function
+ this_arg: The this parameter of the function func
+ ...args: The parameters of the function func

## Execution
The fiber object implements iterator-related properties (next(), end, value), and the caller can execute a fiber through iterator-like operations.
```
//执行纤程
for fiber as r {
}
```
When we create a fiber, the OX virtual machine starts running the function corresponding to the fiber. When it executes a "yield" command, the virtual machine pauses the execution of the fiber and sets its "value" property to the return value of "yield".
When we call the "next()" method of the fiber, the virtual machine resumes execution from the paused position until it reaches the next "yield" command. The virtual machine then pauses the fiber's execution again and sets its "value" property to the return value of this "yield".
We repeatedly call the fiber's "next()" method until the function executes a "return" statement. At this point, the virtual machine sets the fiber's "end" property to true and its "value" property to the return value of the function.
## yield
The syntax description of "yield" is:
```
yield_expression: "yield" expression?
```
The "yield" command can take an expression as the temporary return value of the fiber. If there is no such expression, null is used as the temporary return value.

The "next()" method can also take a parameter, which is the value of the "yield" expression. If the "next()" method has no parameter, the value of the "yield" expression is null.
For example:
```
fiber = Fiber(func {
    stdout.puts("{yield 1}\n") //print 100
    stdout.puts("{yield 2}\n") //print 200
    return 3
})

stdout.puts("{fiber.value}\n") //print 1
fiber.next(100)
stdout.puts("{fiber.value}\n") //print 2
fiber.next(200)
stdout.puts("{fiber.value}\n") //print 3
```
Running the above script will print:
```
1
100
2
200
3
```
The "yield" command can also be added to functions called by the fiber function. However, note that there must be no native functions between the function containing the "yield" command and the fiber function, meaning the fiber function must call the function with "yield" through an OX function.