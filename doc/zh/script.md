# 脚本
## 运行脚本
我们编写一个脚本文件"test.ox"：
```
//test.ox

sum = 0

for i = 1; i < argv.length; i += 1 {
    sum += Number(argv[i]) //将参数转化为数字累加
}

return sum
```
用下命令执行这个脚本：
```
ox --pr test.ox 1 2 3 4 5 6 7 8 9 10
```
选项"--pr"表示打印脚本执行的返回值。打印结果：
```
55
```
当运行脚本时，OX解释器解析编译"test.ox"，并生成一个脚本对象。脚本对象中包含一个入口函数，其代码对应脚本中的语句。
然后OX解释器调用这个入口函数，并将命令行中的"test.ox 1 2 3 4 5 6 7 8 9 10"变为一个字符串数组["test.ox", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"],
将其作为参数传递给入口函数。入口函数执行完毕，将返回值返回为OX解释器。因为运行时带了"--pr"选项，解释器打印了这个结果。

运行脚本的过程如下：

* 编译脚本，生成脚本对象
* 如果脚本对象包含对其他脚本的引用，对每一个引用执行引用过程。
* 运行脚本的入口函数

## 引用
脚本中可以通过引用语句引用其他脚本中定义的功能。

我们创建一个文件"test.ox"，在文件中定义一个函数常量"test"。注意"test"前增加"public"关键字，表示这个符号可以被其他脚本引用。
```
//test.ox
ref "std/io"

public test: func() {
    stdout.puts("test!\n")
}

public test1: func() {
    stdout.puts("test1!\n")
}
```
我们再创建一个文件"run.ox"，在文件中添加引用语句，引用"test.ox"中的函数"test"：
```
//run.ox
ref "./test" test as t //引用"test"，保存为本地符号"t"

t() //调用"test"函数
```
引用语句中"./test"为引用脚本名。注意脚本名不要叫".ox"后缀，OX解释器会自动查找补齐文件名。脚本名有以下几种形式：

|形式|例子|说明|
|:-|:-|:-|
|相对路径|"./test" "../test"|引用脚本相对当前脚本的相对路径名|
|绝对路径|"/home/ox/test" "c:/test"|引用脚本的绝对路径名|
|软件包中库脚本|"std/io"|引用软件包中的的一个库脚本|
|软件包|"std"|引用软件包中全部库脚本|

在引用语句中关键字"as"前为引用符号标识符，代表要从引用脚本中引入的公共符号的名字。
关键字"as"后为本地符号标识符，代表本地脚本中创建的符号名字。注意这个名字不要和本地脚本中定义的变量/常量名冲突，否则会报错。

如果我们引用符号和本地符号名相同，我们可以省略"as 本地符号"部分。上面的代码可以改为：
```
//run.ox
ref "./test" test //引用"test"，保存为本地符号"test"

test() //调用"test"函数
```
如果我们想从一个脚本中引入多个符号，可以在一对大括号中表示:
```
ref "./test" {test, test1 as t1}

test()
t1()
```
如果我们想将脚本中所有符号同时引入，可以写为：
```
ref "./test" //引入"test.ox"中的所有公共符号，创建内部符号"test", "test1"

test()
test1()
```
或者我们可以用访问对象属性的方式访问脚本中的符号：
```
ref "./test" * as t //创建内部符号"t", t保存脚本"test.ox"，"test", "test1"为对象t的属性。

t.test()
t.test1()
```
引用过程OX解释器操作如下：

* 如果引用为一个绝对或相对路径名，代表引用的是一个脚本文件，运行这个脚本文件，得到脚本对应的对象
* 如果引用为一个软件包，代表通过包管理器引用
    * 通过包管理器加载软件包信息，如果没有找到这个软件包抛出异常
    * 如果引用中包含了脚本名，检查软件包中是否包含这个库脚本
        - 如果没有这个库抛出ReferenceError异常
        - 如果找到库脚本，运行这个脚本文件，得到脚本对应的对象
    * 如果引用中不包含脚本名，代表引用软件包中包含的所有库
        - 创建一个脚本对象
        - 依次运行软件包中的所有库脚本，将库脚本中的公共符号加入新建脚本对象中
* 导入脚本对象中的符号
    * 如果引用符号名为"*"并指定了本地名n，表示引入脚本对象。
        - 创建本地符号n，赋值为脚本对象
    * 如果引用符号名不为"*"，查找脚本对象中是否包含引用符号
        - 没有找到引用符号则抛出ReferenceError异常
        - 找到引用符号，则创建本地符号n，赋值为引用符号的值。n为引用中指定的本地符号名，如果没有指定本地符号名，则n与引用符号相同
    * 如果不带引用符号名，或引用符号名为"*"且没有指定本地符号，代表引入脚本的所有公共符号
        - 遍历脚本对象中的公共符号，如果本地没有定义这个符号，则在本地创建同名符号，并赋值为脚本对象中符号的对应值

注意引用全部符号时，只会引入本地不存在的符号，不会覆盖本地变量值。比如"test.ox"中定义公共符号"test_value"：
```
public test_value = 1978
```
在另一个脚本中引用"test"中的全部符号：
```
ref "./test"

test_value = 1
```
因为脚本中已经定义了变量"test_value",因此"test.ox"中的符号"test_value"不会被引入。
为了避免这种引入符号和本地变量的冲突，可以通过"as"关键字修改本地符号名：
```
ref "./test" test_value as ext_test_value //ext_test_value = 1978

test_value = 1
```
或者可以指定引入脚本对象,通过脚本对象的属性访问这个符号：
```
ref "./test" * as test //test.test_value = 1978

test_value = 1
```

## 脚本内置方法
脚本内置以下方法，可以通过这些方法遍历脚本中的符号：

|方法|描述|
|:-|:-|
|keys()|创建迭代器，遍历脚本中全部公共符号名称|
|values()|创建迭代器，遍历脚本中全部公共符号的值|
|entries()|创建迭代器，遍历脚本中全部公共符号，遍历返回值为2元素数组，其值为：[符号名称,符号值]|
|$iter()|功能与entries()相同|

如下面的代码，打印了脚本"std/math"中全部公共符号的名称：
```
ref "std/io"
ref "std/math" * as math

for math.keys() as key {
    stdout.puts("{key}\n")
}
```
## 语法描述
```
script: script_items?

script_items: script_items script_item
    | script_item

script_item: statement
    | ref_statement
    | "public" expression

ref_statement: "ref" string_literal ref_declaration?

ref_declaration: "{" ref_items "}"
    | ref_item

ref_items: ref_items "," ref_item
    | ref_item

ref_item: identifier
    | identifier "as" identifier
    | "*"
    | "*" "as" identifier
```