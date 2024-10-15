# 原生模块生成器
如果我们有一个C语言编写的库，希望通过OX语言调用这个库，我们需要编写一个原生模块对这个C库进行封装。
我们可以使用原生模块生成器"oxngen"程序，自动生成这个封装原生模块的C代码。
注意"oxngen"需要调用clang对头文件进行解析生成语法树，使用"oxngen"前请先安装clang。
"oxngen"在解析过成功会根据"build.ox"文件中定义的规则生成原生模块的源代码，用户可以在"build.ox"文件中定义生成规则。
## 例子
如我们有一个C语言实现的库，其头文件"test.h"包含函数定义：
```
extern void test (int a);
```
我们编辑"build.ox"描述原生模块生成规则：
```
{
    ...

    oxngen_targets: {
        //生成原生模块的源文件"test.oxn.c"
        test: {
            input_files: [
                "test.h" //扫描头文件"test.h"
            ]
        }
    }

    ...
}
```
运行"oxngen"生成原生模块的源文件"test.oxn.c":
```
ox -r oxngen -i build.ox
```
运行gcc生成原生模块"test.oxn":
```
gcc -o test.oxn.o -c -fPIC test.oxn.c
gcc -o test.oxn -shared test.oxn.o -lox -lm -lpthread -ldl -lffi
```
## 生成代码
"oxngen"可以对头文件中的宏，枚举，数据结构，变量，函数生成相应的封装代码。
### 宏
头文件中值为数值或字符串的宏可以生成对应OX代码。如头文件定义：
```
#define M_NUM 1
#define M_STR "hello"
```
"oxngen"会生成常量"M_NUM"和"M_STR"。可在OX脚本中引用生成模块的这两个符号：
```
ref "test" //引用"oxngen"生成模块

stdout.puts("{M_NUM}\n") //打印  1
stdout.puts("{M_STR}\n") //打印 "hello"
```
### 枚举
头文件定义枚举类型：
```
enum {
    E_ITEM_0,
    E_ITEM_1,
    E_ITEM_2
};
```
"oxngen"会生成数值型常量"E_ITEM_0","E_ITEM_1"和"E_ITEM_2"。可在OX脚本中引用生成模块的这些符号：
```
ref "test" //引用"oxngen"生成模块

stdout.puts("{E_ITEM_0} {E_ITEM_1} {E_ITEM_2}\n") //打印 0 1 2
```
### 数据结构
头文件定义数据结构：
```
typedef struct {
    int a;
    int b;
} MyStruct;
```
"oxngen"会生成数据结构对象"MyStruct"。通过调用这个对象，可以分配数据结构实例或数据结构数组。
可在OX脚本中引用生成模块的这些符号：
```
ref "test" //引用"oxngen"生成模块

s = MyStruct() //分配一个数据结构，返回指向MyStruct数据结构的指针。
s.{
    a: 1 //设置s.a = 1
    b: 2 //设置s.a = 2
}

sa = MyStruct(4) //分配一个数据结构数组，返回指向数组的指针
for i = 0; i < C.get_length(sa); i+=1 {
    sa[i] = s //设置数组元素
}
```
### 变量
头文件定义变量：
```
int my_var = 1;
```
"oxngen"会生成对象"my_var",通过对象的"get"和"set"方法，可以对变量进行读取和设置。可在OX脚本中引用生成模块的这些符号：
```
ref "test" //引用"oxngen"生成模块

stdout.puts("{my_var.get()}\n") //打印 1

my_var.set(1978) //设置变量

stdout.puts("{my_var.get()}\n") //打印 1978
```
### 函数
头文件声明函数：
```
int my_func(int a, int *p);
```
"oxngen"会生成C函数对象"my_func"。可在OX脚本中调用这个函数：
```
ref "test" //引用"oxngen"生成模块

i = Int32()

r = my_func(100, &i) //调用函数
```
在生成函数的封装代码里，会对参数进行自动的类型转换。

* 参数为数值类型(char/int/float/double...),如果输入OX数值，会自动转化为相应的C类型数值。
* 参数为字符串，如果输入OX字符串，会自动转化为C字符串。注意如果函数中会修改字符串中的数据，不要传递OX型字符串做参数，因为OX型字符串是不可修改的。

## 生成规则
如果需要使用"oxngen",在"build.ox"中增加"oxngen_targets"属性，其值为一个对象，对象的每个属性名为要生成的原生模块名。
如属性"test",代表生成模块"test.oxn",对应生成的源文件为"test.oxn.c"。
属性值对象为生成这个模块所使用的规则。
### 输入文件
属性"input_files",代表要输入的头文件。其值为一个字符串数组，每个元素表示一个输入头文件。如：
```
input_files: [
    "my_module.h"
    "my_mudule_helper.h"
]
```
### 编译和链接选项：
属性"cflags"表示模块的编译选项，"libs"表示模块的链接选项。如：
```
    cflags: "-I/usr/include/my_module -DVERSION=20260101"
    libs: "-lmy_module"
```
### 文件过滤器
由于头文件中通过"#include"包含了很多其他的头文件，可以选择只对一部分头文件中的定义生成封装代码。如：
```
    file_filters: [
        "my_module.h"
        "my_mudule_helper.h"
    ]
```
上面的定义代表，只对头文件"my_module.h"和"my_mudule_helper.h"中定义的内容生成相应封装代码。

如果只想跳过一些头文件，可以定义反向过滤：
```
    rev_file_filters: [
        "stdio.h"
    ]
```
上面的定义代表，跳过"stdio.h"中的定义。
### 定义过滤器
有时我们只想对头文件中的部分定义生成OX封装代码，我们可以通过定义过滤器实现此功能。如：
```
    decl_filter: [
        "test_func"
        /TEST_.+/p
    ]
```
定义过滤器为一个数组，其元素可以是字符串或增则表达式，如果定义名与元素匹配时，才对这个定义生成相应封装代码。
如用上面的定义过滤器，扫描头文件：
```
enum {
    TEST_1,
    TEST_2
};

void test_func(void)
```
会生成枚举值"TEST_1","TEST_2"和函数"test_func"对应的封装。

如果想跳过某些定义，可通过反向定义过滤器"rec_decl_filters"。如：
```
    rev_decl_filters: [
        "TEST_2"
    ]
```
对上面的头文件扫描，枚举值"TEST_2"就不会生成对应的封装。
### 宏常量
有时我们希望对头文件中通过宏定义的数值或字符串型常量其进行封装。
我们可以通过"number_macros"和"string_macros"进行描述：
```
    number_macros: [
        "MY_PI"
        /NUM_,+/p
    ]
    string_macros: [
        "VERSION_STRING"
    ]
```
"number_macros"和"string_macros"的元素可以是字符串或正则表达式。用上面的定义扫描头文件：
```
#define MY_PI   3.1415926
#define NUM_ITEM1 1
#define NUM_ITEM2 (NUM_ITEM1 + 1)

#define VERSION_STRING "1.0.0"
```
会生成常量"MY_PI", "NUM_ITEM1", "NUM_ITEM2"和"VERSION_STRING"。
### 引用
如果模块中包含了对其他模块的引用，通过"references"进行定义。如：
```
    references: {
        "std/socket": [
            "sockaddr"
        ]
    }
```
"references"值为一个数组，其中每个属性的名字代表引用的模块名，属性值为一个数组，表示引用模块内公共符号的名称。
上面的定义代表模块引用了"std/socket"中定义的符号"sockaddr"。