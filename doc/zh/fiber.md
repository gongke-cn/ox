# 纤程
"std/fiber"库提供了纤程(协程)Fiber类的支持。
## 创建
通过以下语句创建一个纤程：
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
Fiber调用参数为：

+ func: 线程执行函数
+ this_arg: 函数func的this参数
+ ...args: 函数func的参数

## 执行
纤程对象实现了迭代器的相关属性(next(), end, value)，调用者可以通过类似迭代器的操作执行一个纤程。
```
//执行纤程
for fiber as r {
}
```
当我们创建一个纤程时，OX虚拟机开始运行纤程对应的函数，当执行到其中一个"yield"命令时，虚拟机会暂停纤程的执行，并将其"value"属性设置为"yield"的返回值。
当我们调用纤程的"next()"方法时，虚拟机从暂停处继续执行，直到执行到下一个"yield"命令，虚拟机再次暂停纤程的执行，并将其"value"属性设置为"yield"的返回值。
我们反复调用纤程的"next()"方法，直到函数执行到"return"语句，虚拟机将纤程的"end"属性设置为true, 并将其"value"属性设置为函数的返回值。
## yield
"yield"的语法描述为：
```
yield_expression: "yield" expression?
```
"yield"命令可以带一个表达式的作为纤程的临时返回值，如果没有这个表达式，则使用null作为临时返回值。

调用"next()"方法，也可以带一个参数，这个参数是"yield"表达式的值。如果"next()"方法没有带参数，则"yield"表达式的值为null。
如：
```
fiber = Fiber(func {
    stdout.puts("{yield 1}\n") //打印 100
    stdout.puts("{yield 2}\n") //打印 200
    return 3
})

stdout.puts("{fiber.value}\n") //打印 1
fiber.next(100)
stdout.puts("{fiber.value}\n") //打印 2
fiber.next(200)
stdout.puts("{fiber.value}\n") //打印 3
```
上面脚本运行会打印：
```
1
100
2
200
3
```
纤程函数调用的函数中也可以加入"yield"命令。但是注意包含"yield"命令的函数和纤程函数中间不能跨越原生函数，即纤程函数必须是通过OX函数调用带"yield"的函数。