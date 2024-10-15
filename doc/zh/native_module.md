# 原生模块
除了用OX语言编写脚本，我们也可以用C语言编译生成二进制库供OX语言调用。
这种用二进制库形式可供OX语言调用的模块称为原生模块。

原生模块实际是一个动态链接库，其后缀为".oxn"。其中包含以下两个符号：

* ox_load: 加载模块时被调用。
* ox_exec: 模块入口函数。

## ox_load
```
OX_Result ox_load (OX_Context *ctxt, OX_Value *s);
```
在加载模块后，OX解释器会调用这个函数。在这个函数中通常的操作是添加对其他脚本的引用并设置本模块的公共符号表。

## ox_exec
```
OX_Result ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv);
```
此函数为模块脚本的入口函数。在这个函数中一般会创建相关类，对象和函数，并初始化公共符号表。

## 例子
下面的例子实现了一个原生模块。其中模块引用了"std/io"中的"FILE"符号。实现了一个名为"test"的函数。
```
#include <ox.h>

//模块引用数组
static const OX_RefDesc
ref_tab[] = {
    //{模块名, 引用符号名，本地符号名}
    {"std/io", "FILE", "FILE"}, //引用"std/io"中的符号"FILE",本地符号名称为"FILE"
    {NULL, NULL, NULL}
};

//公共符号名数组
static const char*
pub_tab[] = {
    "test", //函数"test"
    NULL
};

//符号索引数组。
//符号最前面应该是公共符号索引，其顺序要与公共符号名数组pub_tab中定义一致。
//后面跟引用的本地符号索引，其顺序要与模块引用数组ref_tab中定义一致。
//最后可以加一些本地定义的内部符号。
enum {
    ID_test, //函数“test”
    ID_FILE, //从"std/io"引用的符号"FILE"
    ID_MAX
};

//脚本基本描述
static const OX_ScriptDesc
script_desc = {
    ref_tab, //模块的引用
    pub_tab, //所有公共符号名
    ID_MAX   //全部符号数目符号
};

//模块加载
OX_Result
ox_load (OX_Context *ctxt, OX_Value *s)
{
    //设置模块脚本基本信息，分配公共符号表
    ox_script_set_desc(ctxt, s, &script_desc);
    return OX_OK;
}

//test函数实现
static OX_Result
test_wrapper (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    //获取符号"FILE"
    OX_Value *file = ox_script_get_value(ctxt, f, ID_FILE);
    ...
    return OX_OK;
}

//模块执行入口函数
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_value_stack_push(ctxt);

    //创建函数"test"
    ox_named_native_func_new_s(ctxt, v, test_wrapper, NULL, "test");
    //将函数"test"绑定到公共符号"test"
    ox_script_set_value(ctxt, s, ID_test, v);

    ox_value_stack_pop(ctxt, v);
    return OX_OK;
}
```
运行以下命令编译生成本地模块"test.oxn"：
```
gcc -o test.o -c -fPIC test.c
gcc -o test.oxn -shared test.o -lox -lm -lpthread -ldl -lffi
```
编写脚本调用"test.oxn":
```
ref "./test"

test()
```