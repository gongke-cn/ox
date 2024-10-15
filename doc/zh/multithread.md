# 多线程
OX语言通过"std/thread"库支持多线程。
## 线程
"std/thread"库中实现了线程"Thread"类。
"Thread"类包含以下方法：

* $init: 线程实例初始化，创建新线程时自动调用
    - 参数
        + func: 线程执行函数
        + this_arg: 函数func的this参数
        + ...args: 函数func的参数
* detach: 将线程转化为detached线程
* join: 等待线程结束。
    - 返回值: 线程函数的返回值

线程分两种类型，"joinable"的"detached"。线程创建时缺省类型为"joinable"，当用户调用"detach()"方法后变为"detached"。
如果线程类型为"joinable"，则该线程的创建者需要调用"join()"方法等待线程结束并释放资源。
如果线程类型为"detached"，线程结束后会自动释放资源，创建者无需调用"join()"方法。

看如下示例创建线程：
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
## 调度
在OX虚拟机中有一个全局锁，为了防止多线程运行时出现竞争，OX虚拟机在运行时会打开全局锁，此时其他OX线程无法运行。

OX虚拟机在运行一些耗时较长且不会引发竞争的操作时，会执行调度运行操作：

* 释放全局锁
* 运行具体操作。此过程中其他线程都有机会执行。
* 打开全局锁

一些标准库中的函数内部是调度运行的，如：

* "std/thread"中Thread.$inf.join方法
* "std/system"中msleep函数
* "std/io"，"std/fs"，"std/system"等库中的函数

如果用户在自己的代码执行过程中允许其他线程运行，可以通过"sched"语句在代码中插入调度点：
```
th[i] = Thread(func(id) {
    for i = 0; i < 100; i ++ {
        sum += i
        sched //加入调度点
    }

    return sum
}, null, i)
```
当运行到"sched"命令时，OX虚拟机会释放全局锁，尝试进行线程调度，等当前线程重新获得CPU是，再加全局锁继续运行下面的操作。
另外"sched"语句可以跟一个语法块, 如：
```
sched {
    my_func_1()
    my_func_2()
    my_func_3()
}
```
以上的语句表示，如果语法块中有C函数调用，则这些C函数调用都是线程安全的，都会通过调度运行操作来进行调用。

语法描述：
```
sched_statement: "sched" ("{" statements? "}")?
```
## 消息队列
推荐使用"std/queue"库中的消息队列进行多线程通讯。
"std/queue"库提供消息队列"Queue"类。其包含以下方法：

* $init: 消息队列初始化，创建时自动调用
* send: 向消息队列发送消息值
    - 参数
        + msg: 向消息队列发送的消息值
* wait: 等待直到收到消息
    - 参数
        + timeout: 毫秒为单位的超时时间，如没有指定表示一直等待
    - 返回值: 收到的消息值。如果等待超时返回null
* check: 检查消息队列是否收到消息
    - 返回值: 如果收到消息返回收到的消息值，如没收到消息返回null

示例：
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