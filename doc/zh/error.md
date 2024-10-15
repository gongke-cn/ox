# 异常
当程序运行错误时，OX程序通常会通过"throw"语句抛出一个异常对象。
OX语言内部定义了以下异常对象类：

|名称|描述|
|:-|:-|
|Error|通用错误，是其他异常对象的父类|
|NullError|期望一个有效值但实际值为null|
|TypeError|数据类型不匹配|
|RangeError|超出有效范围|
|SystemError|系统错误|
|ReferenceError|无效的引用错误|
|NoMemoryError|没有足够的内存空间|
|TypeError|数据类型不匹配|
|AccessError|没有访问权限|
|SyntaxError|语法错误|

## Error
“Error”是其他错误的父类。其定义了以下方法：

* $init: 异常对象实例初始化函数，创建异常对先实例时自动调用。
    + 参数：
        - message: 错误信息字符串。此字符串被保存在异常实例的"message"属性中。
* $to_str: 将异常对象实例转化为字符串。字符串格式为"异常名: 错误信息"。

## 自定义异常
用户可以在程序中定义自己的异常。如：
```
//自定义异常，继承"Error"类
MyError: class Error {
}

try {
    ...
} catch e {
    if e instof MyError {
        // 捕获自定义异常
    }
}
```
用户也可以在异常对象中增加自己定义的属性：
```
//自定义异常，继承"Error"类
MyError: class Error {
    //除了错误信息，异常中还保存文件名和行号
    $init(msg, file, line) {
        //初始化Error基本信息
        Error.$inf.$init(this, msg)
        //保存文件名和行号
        this.{
            file
            line
        }
    }

    //字符串转化函数
    $to_str() {
        s = Error.$inf.$to_str(this)

        //增加文件名和行号信息
        return "{file}: {line}: {s}"
    }
}
```