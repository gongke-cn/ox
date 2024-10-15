# Hello World
来，开始编写你的第一个OX脚本！
打开文本编辑器，编写如下代码：
```ox
ref "std/io"

stdout.puts("hello, world!\n")
```
将这个脚本保存为"hello_world.ox"。然后，运行以下命令运行这个脚本：
```shell
ox hello_world.ox
```
我们看到终端打印如下：
```shell
hello, world!
```
我们来看一下这个脚本到底做了什么。第一行:
```ox
ref "std/io"
```
这是OX语言的引用语句，说明这段脚本引用了另一个脚本中定义的功能。
`std/io`是引用脚本的名字。其中前面的`std`是软件包的名字,`io`是软件包中脚本文件的名字,软件包和脚本文件名称用`/`进行分隔。
这条语句的意思是当前脚本引用了OX语言的标准软件包(`std`)中的输入输出模块(`io`)脚本。

下一条语句:
```ox
stdout.puts("hello, world!\n")
```
`stdout`是脚本`"std/io"`定义的标准输出对象，`puts`代表标准输出对象的打印字符串方法。括号中的"hello, world!\n"是传递给`puts`方法的参数。
这行语句的意思是，调用标准输出对象的打印字符串方法，向标准输出打印`"hello, world!"`字符串。
