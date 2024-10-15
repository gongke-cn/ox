# 示例
## 终端打印
向终端打印“hello, world!”。
```
//引用标准库的输入输出库
ref "std/io"

//向标准输出打印
stdout.puts("hello, world!\n")
```
## 文本处理
将文本文件中所有"NAME"替换为“张三”。
```
//引用标准库的输入输出库
ref "std/io"

//打开文件
#file = File("input.txt", "rb")

//遍历文件中的所有行
for file.$iter(($.gets())) as line {
    //将"NAME"替换为“张三”
    nline = line.replace(/\bNAME\b/, "张三")
    //输出替换结果
    stdout.puts(nline)
}
```
## 调用C接口
调用cURL库进行HTTP通讯。
```
//引用cURL模块
ref "curl"
//引用标准库的输入输出库
ref "std/io"

//cURL全局初始化
curl_global_init(CURL_GLOBAL_DEFAULT)

//创建cURL easy session.
curl = curl_easy_init()

//设定URL
curl_easy_setopt(curl, CURLOPT_URL, "https://baidu.com")

//写数据回调函数
write_fn: func(buf, size, num) {
    stdout.puts("recv: {buf.$to_str(0, num)}\n")
    return num
}.to_c(C.func_type(Size, UInt8.pointer, Size, Size))

//设置写数据回调函数
curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_fn)

//执行HTTP会话
curl_easy_perform(curl)

//释放session
curl_easy_cleanup(curl)

//cURL全局释放
curl_global_cleanup()

```