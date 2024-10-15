# 基本语法
## 语法描述
在OX语法文档中，用语法描述表示具体的语法规则。如：
```
tvalue: "true"
```
以上称为一个非终结符的产生式，冒号左侧为语法定义的非终结符名称，右侧为组成这个非终结符的元素。
当OX编译器检测到源文件中顺序出现右侧的这些元素时，这些元素被简化成左侧的非终结符。
比如上面的产生式表示当检测到源文件中出现字符串"true"时，被简化为tvalue非终结符。

一个非终结符有时可以从多个产生式生成，我们可以在冒号右侧通过"|"进行分隔：
```
bool: "true"
    | "false"
```
上面的规则表示，当源文件中出现字符串"true"或"false"时，可以简化为bool非终结符。

在右侧可以使用"~"表示一个区间内的所有字符，如：
```
digital: "0" ~ "9"
```
上面的规则表示，"0"到"9"所有字符都可以简化为digital非终结符。等价于：
```
digital: "0"
    | "1"
    | "2"
    | "3"
    | "4"
    | "5"
    | "6"
    | "7"
    | "8"
    | "9"
```
如果元素可能出现多次，可以加以下重复标识表示：

|标识|说明|
|:-|:-|
|?|元素出现0次或1次|
|+|元素出现1次或任意多次|
|*|元素出现0次或任意多次|
|{N}|元素出现N次|

比如：
```
digital: "0" ~ "9"

integer: "-"? digital+
```
integer的规则表示，integer前可以出现负号"-"，后根1到多个数字字符。

可以使用小括号对几个元素进行分组，小括号后可以加重复标识表示这组数据的重复次数，如：
```
digital: "0" ~ "9"

number_list: digital+ ("," digital+)*
```
## 标识符
标识符以标识起始字符开始，后面跟0到多个标识后续字符。

标识起始字符可以是："$", "_", "a" ~ "z", "A" ~ "Z"

标识后续字符可以是："$", "_", "a" ~ "z", "A" ~ "Z", "0" ~ "9"

以下都是合法的标识符：
```
a
a0
_my_var_0
$var0
```
注意以下标识符在OX语言中有特殊意义，不要使用这些标识符作为变量名。

|名称|说明|
|:-|:-|
|null|null字面量|
|true false|布尔型字面量|
|this|当前函数的this参数|
|argv|当前函数的参数数组|
|$|当前函数的参数0|
|$N|当前函数的第N个参数。N为十进制数字。|
|标识符型关键字|ref, if, for, do, while, try, catch等|

语法描述：
```
identifier: identifier_first_character identifier_following_character*

identifier_first_character: "$"
    | "_"
    | "a" ~ "z"
    | "A" ~ "Z"

identifier_following_character: "$"
    | "_"
    | "a" ~ "z"
    | "A" ~ "Z"
    | "0" ~ "9"
```

## 关键字
以下标识符在OX语言中用做关键字，不要用这些标识符当作变量或常量名使用。
```
null true false
func class static enum bitfield if elif else while do for as
case break continue return try catch finally throw typeof instof
ref public this argv sched yield global owned textdomain
```