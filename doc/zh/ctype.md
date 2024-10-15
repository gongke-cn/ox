# C类型
OX语言中为了方便和C语言实现的原生函数进行相互调用，提供了C类型的支持。

C类型包含以下几种：

* 基本类型: 包括整数和浮点数数据类型
* 指针类型: 指针和数组
* 函数类型

## 基本类型
OX语言中提供以下C基本数据类型。

|对象|对应C语言数据类型|描述|
|:-|:-|:-|
|Int8|int8_t|8位有符号整数|
|Int16|int16_t|16位有符号整数|
|Int32|int32_t|32位有符号整数|
|Int64|int64_t|64位有符号整数|
|UInt8|uint8_t|8位无符号整数|
|Uint16|uint16_t|16位无符号整数|
|UInt32|uint32_t|32位无符号整数|
|UInt64|uint64_t|64位无符号整数|
|Float32|float|32位浮点数|
|Float64|double|64位双精度浮点数|
|Size|size_t|无符号整数，数值长度等于指针长度|
|SSize|ssize_t|有符号整数，数值长度等于指针长度|
|Void|void||

通过调用C类型对象，可以创建一个对应的C值对象：
```
i8 = Int8() //创建一个8位有符号整数，初始值为0
u32 = Uint32() //创建一个32位无符号整数，初始值为0
f64 = Float() //创建一个64位双精度浮点数，初始值为0
```
如果要修改一个C值，需要对其属性"value"进行赋值:

```
i32 = Int32() //创建一个32位有符号整数，初始值为0
i32.value = 1982 //将32位有符号整数值设置为1982
```
注意不要直接对保存C值的变量赋值，这种操作会将变量重新赋值为一个OX数值，而不再是C值。如：
```
v = Int32() //创建一个32位有符号整数，初始值为0
stdout.puts("{typeof v}\n") //打印"Object:Int32"
v = 1982
stdout.puts("{typeof v}\n") //打印"Object:Number"
```
基本类型支持"$to_num()"方法，可以通过这个方法转化为OX数值。
```
a = Int32()
a.value = 1978
n = a.$to_num() //将无符号整数a转化为OX数值n

n = a + 1 //加法运算隐含调用了a.$to_num(), n的结果为OX数值1979
```
OX数值对象包含以下方法，可以将数值转化为C基础类型：

|方法|返回C类型|
|:-|:-|
|to_int8()|Int8|
|to_int16()|Int16|
|to_int32()|Int32|
|to_int64()|Int64|
|to_uint8()|UInt8|
|to_uint16()|UInt16|
|to_uint32()|UInt32|
|to_uint64()|UInt64|
|to_float32()|Float32|
|to_float64()|Float64|
|to_size()|Size|
|to_ssize()|SSize|

```
a = 1978.to_int32() //a为32位有符号数1978
b = 3.1415926.to_float32() //b为32位浮点数3.1415926
```
## 指针类型
通过"&"运算符，我们可以取一个C类型值的指针。通过"*"运算符，我们可以对一个C指针取其存储的值。
语法描述：
```
get_pointer_expression: "&" expression

get_value_expression: "*" expression
```
如：
```
i32 = Int32()
i32.value = 1978
ptr = &i8 //获取指针
stdout.puts(*ptr) //打印 1978
```
如果调用C类型对象时带一个数值参数，表示创建一个C值数组并返回指向数组的指针：
```
a = Int32(256) //创建一个32位有符号数数组，长度为256
```
对C类型数组元素可以进行设置和读取:
```
a = Int32(256) //创建一个32位有符号数数组，长度为256
for i = 0; i < 256; i += 1 {
    a[i] = i //设置数组元素
}
for i = 0; i < 256; i += 1 {
    stdout.puts("{a[1]} ") //读取数组元素
}

for a as i {
    stdout.puts("{i} ") //遍历打印数组元素
}
```
通过调用函数"C.get_length"可以获取一个C值数组的长度：
```
a = Int32(256)
stdout.puts(C.get_length(a)) //打印 256
```
如果指针对应数组的长度未知，"C.get_length"返回-1。
如果"C.get_length"的参数不是指针，函数抛出TypeError异常。

通过调用函数"C.get_ref"，可以创建一个指向数组内部元素的指针。如：
```
a = UInt8(256)

ref = C.get_ref(a, 200, 1) //ref为指向a[200]的指针
*ref = 0xff //通过ref指针对a[200]赋值

ref = C.get_ref(a, 0, 2) //ref为地址与a相同，长度为2的数组指针
ref[0] = 1 //通过ref指针对a[0]赋值
ref[1] = 2 //通过ref指针对a[1]赋值
```
如果指针类型的元素类型为UInt8或Int8，则可以调用"$to_str()"方法将值转化为OX字符串。
"$to_str()"函数带两个参数：

* pos: 字符串第一个字符在字符数组指针中的索引。缺省为0。
* len: 字符串长度。如果没有此参数，则字符串从pos开始直到一个存放NUL(0)的位置结尾。

如：
```
a = Int8(4)
a[0] = 'a'
a[1] = 'b'
a[2] = 'c'
a[3] = 0

s = a.$to_str(0, 1) //创建字符串"a"
s = a.$to_str(0, 2) //创建字符串"ab"
s = a.$to_str() //创建字符串"abc"
s = a.$to_str(1) //创建字符串"bc"
```
## 函数类型
函数类型内部包括一个返回值类型和一组参数类型。
通过调用函数"C.func_type"可以创建一个函数类型。
"C.func_type"的第一个参数代表函数的返回值类型，其他参数代表函数的参数类型。
```
ftype = C.func_type(Void, Int32, Int32) //创建函数类型 void (*func) (uint32_t, uint32_t)
```
OX编写的函数可以通过调用方法"to_c()"将其转化为C函数，可以用这种方法编写OX函数并将其作为C函数的回调函数使用。
"to_c()"方法的参数为一个函数类型对象，表示要传化为C函数的类型。
如：
```
//my_cb为一个类型为 int32_t (*cb) (int32_t, int32_t) 的函数
my_cb = func(a, b) {
    return a - b
}.to_c(C.func_type(Int32, Int32, Int32))

//将my_cb作为回调函数传递给C函数c_func
c_func(100, my_cb)
```
## C类型属性
C类型都支持以下属性：

|名称|说明|
|:-|:-|
|pointer|获取该类型对应的指针类型|
|size|获取改类型数值占用的空间(字节为单位)。如果类型大小未知，返回-1|
|length|如果类型为C数组，返回数组元素数目。如果数组元素未知，返回-1。如果不是数组，返回1。|

```
stdout.puts(Int8.size) //打印 1
stdout.puts(UInt64.size) //打印 8
stdout.puts(UInt64.pointer.size) //64位系统打印8，32位系统打印4
```