# Multithreading
OX language supports multithreading through the "std/thread" library.
## Threads
The "Thread" class for threads is implemented in the "std/thread" library.
The "Thread" class includes the following methods:

* $init: Initializes the thread instance, automatically called when creating a new thread
    - Parameters
        + func: The execution function of the thread
        + this_arg: The this parameter of the function func
        + ...args: The parameters of the function func
* detach: Converts the thread to a detached thread
* join: Waits for the thread to finish.
    - Return value: The return value of the thread function

There are two types of threads: "joinable" and "detached". The default type of a thread when created is "joinable". When the user calls the "detach()" method, it becomes "detached".
If the thread type is "joinable", the creator of the thread needs to call the "join()" method to wait for the thread to finish and release resources.
If the thread type is "detached", the thread will automatically release resources after ending, and the creator does not need to call the "join()" method.

See the following example to create threads:
```
ref "std/thread"
ref "std/io"

th = []

//创建10个线程
for i = 0; i < 10; i += 1 {
    th[i] = Thread(func(id) {
        ...
    }, null, i)
}

//等待线程结束
for i = 0; i < 10; i += 1 {
    th[i].join()
}
```
## Scheduling
There is a global lock in the OX virtual machine. To prevent race conditions during multithreaded execution, the OX virtual machine will acquire the global lock at runtime, and other OX threads cannot run at this time.

When the OX virtual machine executes some time-consuming operations that do not cause race conditions, it will perform a scheduled execution operation:

* Release the global lock
* Run the specific operation. During this process, other threads have the opportunity to execute.
* Acquire the global lock

Some functions in the standard library are executed in a scheduled manner, such as:

* The Thread.$inf.join method in "std/thread"
* The msleep function in "std/system"
* Functions in libraries such as "std/io", "std/fs", "std/system"

If the user allows other threads to run during the execution of their own code, a scheduling point can be inserted in the code through the "sched" statement:
```
th[i] = Thread(func(id) {
    for i = 0; i < 100; i ++ {
        sum += i
        sched //加入调度点
    }

    return sum
}, null, i)
```
When the "sched" command is executed, the OX virtual machine will release the global lock and attempt thread scheduling. When the current thread regains the CPU, it will re-acquire the global lock and continue to run the subsequent operations.
In addition, the "sched" statement can be followed by a syntax block, for example:
```
sched {
    my_func_1()
    my_func_2()
    my_func_3()
}
```
The above statement means that if there are C function calls in the syntax block, these C function calls are thread-safe and will all be called through the scheduled execution operation.

Syntax description:
```
sched_statement: "sched" ("{" statements? "}")?
```
## Message Queue
It is recommended to use the message queue in the "std/queue" library for multithreaded communication.
The "std/queue" library provides the "Queue" class for message queues. It includes the following methods:

* $init: Initializes the message queue, automatically called when created
* send: Sends a message value to the message queue
    - Parameters
        + msg: The message value sent to the message queue
* wait: Waits until a message is received
    - Parameters
        + timeout: Timeout time in milliseconds; if not specified, it means waiting indefinitely
    - Return value: The received message value. Returns null if the wait times out
* check: Checks whether a message is received in the message queue
    - Return value: Returns the received message value if a message is received, or null if no message is received

Example:
```
ref "std/thread"
ref "std/queue"
ref "std/io"

queue = Queue()

th = Thread(func {
    v = queue.wait()
    stdout.puts("recv {v}\n")
})

for i = 0; i < 100; i +=1 {
    queue.send(i)
    sched
}

th.join()
```