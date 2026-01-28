# 语句
## 语法描述
```
statement: if_statement
    | case_statement
    | while_statement
    | do_while_statement
    | for_statement
    | for_as_statement
    | try_statement
    | break_statement
    | continue_statement
    | return_statement
    | expression
    | sched_statement

statements: statements statement
    | statement
```
## 语句分隔符
在OX语言中，两条语句中间可以通过分号";"进行分隔。如：
```
a=1; b=2; c=3
```
## 换行
如果在两条语句中换行，则语句分隔符";"可以省略。如：
```
a=1;
b=2;
c=3
```
等价于：
```
a=1
b=2
c=3
```
另外，一条较长的语句也可以分为多行。
OX解释器在读到一行末尾最后一个符号时，如果按照语法，这个符号不能作为语句结束标识，则OX解释器继续读取下面行中的符号进行解析。
如果行末尾最后一个符号可以作为语句结束标识，则OX解释器会自动认为当前语句已经结束。
如：
```
a = 1 //"a=1"可以为一条有效语句，此行语句已经结束
a = 1 + //"+"不能作为语句最后一个符号，继续读取下一行
  2     //此处语句结束, 语句等价于 a = 1 + 2
```
## 分支语句
### if语句
将表达式值转化为布尔值，如果布尔值为真，执行代码块：
```
//if ...
if a == 1 {
    stdout.puts("ok\n")
}

//if ... elif ...
if a == 1 {
    stdout.puts("1\n")
} elif a == 2 {
    stdout.puts("2\n")
}

//if ... elif ... else ...
if a == 1 {
    stdout.puts("1\n")
} elif a == 2 {
    stdout.puts("2\n")
} elif a == 3 {
    stdout.puts("3\n")
} else {
    //以上条件都不满足时执行
    stdout.puts("other\n")
}
```
if语句也可以作为表达式使用，表达式的值对应实际执行的代码块中最后一个语句的值。如：
```
v = if a == 1 {
    stdout.puts("a == 1\n")
    1000 //a==1时设v=1000
} else {
    stdout.puts("other\n")
    2000 //a!=1时设v=2000
}
```
if语句还可以用在数组和对象的定义中，用来在特定条件下加入数组元素和对象属性。如：
```
a = [
    0
    1
    //当cond为true时，加入数组元素2,3
    if cond {
        2
        3
    }
]

o = {
    a: 1
    b: 2
    if cond {
        c: 3 //cond为true时，设置属性c为3
    } else {
        c: 4 //cond为false时，设置属性c为4
    }
}
```
语法描述：
```
if_statement: "if" expression "{" statements? "}" ("elif" expression "{" statements? "}")* ("else" "{" statements? "}")?
```
### case语句
判断一个表达式的值，将这个值和分支条件比较，比较结果为真时执行对应代码块，分支条件"*"表示不满足其他条件时执行此代码块：
```
case a {
1 {
    //a == 1
    stdout.puts("1\n")
}
2
3 {
    //a == 2 或 a == 3
    stdout.puts("2 or 3\n")
}
* {
    //以上条件都不满足时执行
    stdout.puts("other\n")
}
}
```
一般情况下case比较表达式的值和分支条件的值是否相等，相等时再执行对应代码块。
case语句还可以用一个复杂表达式做分支条件，当表达式返回值转化为布尔值为真时，再执行对应代码块。
```
case a {
($ > 3)
($ < 5) {
    //a > 3 && a < 5
    stdout.puts("a > 3 && a < 5\n")
}
($ > 2) {
    //a > 2
    stdout.puts("a > 2\n")
}
($ > 1) {
    stdout.puts("a > 1\n")
}
* {
    stdout.puts("other\n")
        c: 4  
}
}
```
分支条件为一对小括号包围的表达式，表示采用计算表达式方式进行分支比较，表达式中"$"代表case用来比较的表达式"a"的值。

case语句也可以作为表达式使用，表达式的值对应实际执行的代码块中最后一个语句的值。如：
```
//当a大于0时，v为1, 当a小于0时, v为-1, 当a等于0时，v为0
v = case a {
($ > 0) {
    1
}
($ < 0) {
    -1
}
0 {
    0
}
}
```
case语句还可以用在数组和对象的定义中，用来在不同条件下加入不同的数组元素和对象属性。如：
```
a = [
    case cond {
    0 {
        "a" //cond == 0时加入元素"a"
    }
    1 {
        "b" //cond == 1时加入元素"b"
    }
    * {
        "c" //其他情况下加入元素"c"
    }
    }
]

o = {
    a: 1
    case cond {
    0 {
        b: 0 //cond == 0时加入属性b
    }
    1 {
        c: 1 //cond == 1时加入属性c
    }
    * {
        d: 2 //其他情况下加入属性d
    }
    }
}
```
语法描述：
```
case_statement: "case" expression "{" case_items? "}"

case_items: case_conditions "{" statements? "}"

case_conditions: case_conditions case_condition
    | case_condition

case_condition: "*"
    | expression
    | "(" expression ")"
```
## 循环语句
循环语句包括"while"语句, "do while"语句, "for"语句, "for as"语句。
### while语句
当满足循环条件时，执行代码块。
```
//打印0 ~ 99
a = 0
while a < 100 {
    stdout.puts("{a}\n")
    a += 1
}
```

语法描述：
```
while_statement: "while" expression "{" statements? "}"
```
### do while语句
do while 循环先执行代码块，再检测循环条件。如：
```
//先执行再检测条件，结果打印"0"
a = 0
do {
    stdout.puts("{a}\n")
} while a > 1
```

语法描述：
```
do_while_statement: "do" "{" statements? "}" "while" expression
```
### for语句
```
for i = 0; i < 100; i += 1 {
    stdout.puts("{i}\n")
}
```
看上面的代码，for的循环条件分为三部分，通过";"进行分隔。
第一部分为初始化部分，在进入循环前执行一次。
第二部分为循环检测条件，当每次准备运行循环内代码时执行，如果检测条件为假立即退出循环。
第三部分为数据更新部分，每次执行完循环内代码时运行。
这三部分都是可选的。如果第二部分循环检测条件为空，表示检测条件一直为真。

```
//死循环
for ;; {
    stdout.puts("loop\n")
}
```

语法描述：
```
for_statement: "for" expression? ";" expression? ";" expression? "{" statements? "}"
```
### for as语句
for...as语句表示创建迭代器，循环遍历迭代器中的所有值。
```
a = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]

//创建迭代器，遍历数组中的全部元素。
for a as i {
    stdout.puts("{i}\n")
}
```
执行过程如下：

* "as"左侧为一个表达式，调用表达式的"$iter()"属性，创建一个新的迭代器。
* 循环
    1. 检测迭代器的"end"属性，如果end为真，立即退出循环
    2. 访问迭代器的"value"属性获取迭代器当前值
    3. 将迭代器当前值复制给"as"右侧的表达式
    4. 执行代码块
    5. 跳转到步骤1
* 调用迭代器的"$close()"属性关闭迭代器

语法描述为：
```
for_as_statement: "for" expression "as" reverse_assginment_left_value "{" statements? "}"
```
### break语句
break语句可以出现在循环语句中，执行此语句后立即跳出外层循环。如：
```
i = 0
while true {
    if i > 100 {
        //打印100次后，跳出循环
        break
    }
    stdout.puts("{i}\n")
    i += 1
}
```
### continue语句
continue语句可以出现在循环语句中，执行此语句后立即结束循环体中代码执行进入下一次循环。如：
```
//打印0, 2, 4, 6, ... 98
for i = 0; i < 100; i += 1 {
    if i & 1 {
        //如果i为奇数，进入下一次循环
        continue
    }
    stdout.puts("{i}\n")
}
```
语法描述为：
```
continue_statement: "continue"
```
## throw语句
抛出异常，OX解释器立即结束当前执行流程，直接跳转到外层"try"语句中的"catch"块，此时异常被捕获，执行"catch"块中对应的代码。
如果抛出的异常没有被捕获，OX解释器直接退出并打印错误和堆栈信息。
```
//抛出类型错误
throw TypeError("type error!")
```

语法描述为：
```
throw_statement: "throw" expression
```
## try语句
```
try {
    do_something()
} catch e {
    stdout.puts("error!\n")
}
```
try代码块执行时，如果有异常抛出，立即进入catch块，抛出的异常被赋值给变量"e"，然后执行catch块中代码打印错误。
如果try代码块执行正常没有产生异常，则忽略 catch块部分。

如果try语句需要在异常和正常情况下都需要执行一些相同的操作，可以在try语句末尾添加finally块。
```
a = File()

try {
    do_something()
} finally {
    a.$close()
}
```
上面的代码在finally块中增加了对文件a的关闭操作。则不管do_something()执行是否产生异常，文件a都会被关闭。
如果do_something()中抛出异常，OX先跳转到finally中执行文件a的关闭操作，然后继续向上层抛出异常。

语法描述为：
```
try_statement: "try" "{" statements? "}" ("catch" reverse_assginment_left_value "{" statements "}")? ("finally" "{" statements "}")?
```
## return语句
直接跳出当前函数，并给出函数返回值。如果"return"语句之后没有加表达式，表示返回值为null。
```
func f1(a,b) {
    //返回参数a+参数b的值
    return a + b
}

stdout.puts("{f1(1, 2)}\n") //打印3

func f2 {
    //返回null
    return
}

stdout.puts("{f2()}\n") //打印空字符串
```

语法描述为：
```
return_statement: "return" expression?
```