# 字面量
OX语言中每个字面量表示一个值。
## null
“null”是一个表示空值的特殊字面量。在OX语言中，如果一个变量没有被设置，其缺省值即为"null"。

语法描述：
```
null_literal: "null"
```
## 布尔型字面量
OX语言中包含两个布尔型字面量，"true"和"false",分别对应布尔的真和假。

语法描述：
```
bool_literal: "true"
    | "false"
```
## 数字型字面量
OX语言中数字值符合"IEEE754"双精度数标准。一个数字值以64位存储。
OX语言中数字型字面量有以下几种形式。

语法描述：
```
number_literal: decimal_integer_literal
    | binary_integer_literal
    | octal_integer_literal
    | hexadecimal_integer_literal
    | decimal_number_literal
    | exponential_number_literal
```
### 十进制整数
以一到多个字符"0" ~ "9"的序列组成。如：
```
0
1234567890
0123456789
```

语法描述：
```
decimal_integer_literal: digital_character digital_character_or_separator*

digital_character_or_separator: digital_character
    | number_separator

digital_character: "0" ~ "9"

decimal_integer: digital_character+
```
### 二进制整数
以"0b"或"0B"开始，后接一到多个字符"0"或“1”的序列。如：
```
0b11110000
0B01010101
```

语法描述：
```
binary_integer_literal: binary_prefix binary_character_or_separator+

binary_prefix: "0b"
    | "0B"

binary_character_or_separator: "0"
    | "1"
    | number_separator
```
### 八进制整数
以"0o"或"0O"开始，后接一到多个字符"0"~“7”的序列。如：
```
0o76543210
0O01234567
```

语法描述：
```
octal_integer_literal: octal_prefix octal_character_or_separator+

octal_prefix: "0o"
    | "0O"

octal_character_or_separator: octal_character
    | number_separator

octal_character: "0" ~ "7"
```
### 十六进制整数
以"0x"或"0X"开始，后接一到多个字符"0"~“9”, "a"~"f","A"~"F"的序列。如：
```
0offee0123
0O0844AEFF
```

语法描述：
```
hexadecimal_integer_literal: hexadecimal_prefix hexadecimal_character_or_separator+

hexadecimal_prefix: "0x"
    | "0X"

hexadecimal_character_or_separator: hexadecimal_character
    | number_separator

hexadecimal_character: "0" ~ "9"
    | "a" ~ "f"
    | "A" ~ "F"
```
### 小数形式
格式为“整数部分.小数部分”，其中整数和小数部分都是一到多个"0" ~ "9"字符的序列。如：
```
3.1415926
0.0
0.0001234
```
注意和C语言不同，整数或小数部分不能为空，下面的字面量书写是错误的：

```
.123 //错误：整数部分不能为空
123. //错误：小数部分不能为空
```

语法描述：
```
decimal_number_literal: digital_character digital_character_or_separator* "." digital_character_or_separator+
```
### 指数形式
在十进制整数或小数形式后加入"e"或"E"和指数部分。指数部分为一到多个"0"或“1”的字符序列。指数部分前可加入表示符号的"+"或“-”。如：
```
1e10
1e+10
1E-10
12.345e5
```

语法描述：
```
exponential_number_literal: ddigital_character digital_character_or_separator* ("." digital_character_or_separator+)? exponential_flag plus_or_minus? digital_character_or_separator+

exponential_flag: "e"
    | "E"

plus_or_minus: "+"
    | "-"
```
## NAN 和 无限值
"Number.NAN"表示"Not a number",数值中存储的不是一个有效"IEEE754"数值。
"Number.INFINITY"表示无限值。
### 数值分隔符
在书写一个位数较多的数字时，为避免错误，可以在数字字符中插入"\_"进行分隔。"\_"不会影响数值的计算。如：
```
123_456_789
0xfe33_543f_92ab_ec33
```

语法描述：
```
number_separator: "_"
```
## 字符型字面量
OX语言中以一对单引号包围一个字符表示字符型字面量。如：
```
'a'
'0'
'!'
```
字符型字面量的值等于字符的ASCII码数值。比如
```
'a' //等于97
'0' //等于48
'!' //等于33
```
字符型字面量中还可以加入"\"转义字符：

|字符|值|
|:-|:-|
|'\n'|LF 换行|
|'\r'|CR 回车|
|'\t'|HT 水平制表符|
|'\v'|VT 垂直制表符|
|'\f'|FF 换页符|
|'\a'|BEL 响铃符|
|'\b'|BS 回退符|
|'\\'|反斜杠|
|'\''|单引号|
|'\"'|双引号|
|'\xNN|码值为NN的字符，其中N为16进制字符|
|'\uNNNN'|码值为NNNN的unicode字符，其中N为16进制字符|
|'\u{N...}'|码值为NNNN的unicode字符，其中N为16进制字符,N可以有一到多个|

语法描述：
```
character_literal: "'" character_data "'"

character_data: printable_character_except_single_quotation
    | escape_character

printable_character_except_single_quotation: 除"'", "\"以外所有可打印ASCII字符

escape_character: "\" "n"
    | "\" "r"
    | "\" "t"
    | "\" "v"
    | "\" "f"
    | "\" "a"
    | "\" "b"
    | "\" "\"
    | "\" "'"
    | "\" "\""
    | "\" "x" hexadecimal_character{2}
    | "\" "u" hexadecimal_character{4}
    | "\" "u" "{" hexadecimal_character+ "}"
```
## 字符串字面量
字符为0到多个8位字符组成的序列。在OX语言中，字符串以"UTF-8"进行编码。
字符串以一对双引号包围多个字符，双引号中也可以使用"\"转义字符。
如：
```
"" //空字符串
"a"
"hello!"
"line 1\nline2" //包含转义换行符
```
字符串中也可以直接加入换行符表示一个多行字符串：
```
"line 1
line 2
line 3"
```
OX语言中还可以用一对"''"包围多个字符表示字符串。这种形式主要用来更好的表示多行文本。如：
```
''string'' //等价于"string"

''line 1
line 2
'' /*等价于"line 1\nline 2"*/

''
line 1
line 2
line 3
'' /* 等价于"line 1\nline 2\nline 3"*/
```
注意"''"表示多行字符串时，如果起始"''"后这行字符全是空格，这行会被忽略，认为字符串直接从下一行开始，末尾行到结束"''"如果全是空格，这行也会被忽略。

不同于双引号字符串，单引号字符串中不支持转义字符，如：
```
''\n\r\a\b'' //等价于"\\n\\r\\a\\b"
```
为了方便构建复杂的字符串，OX语言可以在字符串中嵌入表达式，动态构建字符串，如：
```
name = "张三"
text = "{name} is a good man."
```
在下面的双引号的字符串字面量中，通过一对大括号"{"和"}"将表达式name嵌入字符串中，这样只要修改name值，就可以替换text对应的字符串值。
在单引号字符串中，是通过"{{"和"}}"嵌入表达式。如：
```
''
{{name}} is my bro!
''
```
在嵌入表达式后可以加入格式描述，表示将表达式按哪种格式转化为字符串。
```
"number: {n!08x}"
```
表达式和格式描述间通过字符"!"进行分隔。格式描述从左到右包含以下几部分：

* 前缀(可省略)：可以为以下几种。如果省略前缀，表示用空格占位且采用右对齐。
    - 0: 数值前自动补0占位
    - -： 左对齐
* 宽度(可省略)：10进制数字，表示表达式转化的字符串的总宽度，如果实际宽度小于这个值，会填充“0”(前缀为“0”时)或空格占位。
* 精度(可省略)：“."后根一个10进制数字，表示浮点数的精度
* 格式：可以为以下几种
    - o: 表达式为无符号整数，按八进制输出
    - d: 表达式为整数，按十进制输出
    - u: 表达式为无符号整数，按十进制输出
    - x: 表达式为无符号整数，按十六进制输出
    - f: 表达式为浮点数，按小数形式输出
    - e：表达式为浮点数，按指数形式输出
    - n：表达式为数值，自动选择十进制整数，小数或指数形式输出
    - c: 表达式为数值，将其视为ASCII码值输出对应的字符
    - s: 表达式为字符串

语法描述：
```
string_literal: "\"" double_quotation_string_item* "\""
    | "''" single_quotation_string_item* "''"

double_quotation_string_item: printable_character_except_double_quotation
    |escape_character
    |single_bracket_string_expression

single_quotation_string_item: printable_character
    |double_bracket_string_expression

printable_character_except_double_quotation: 除"\""以外所有可打印ASCII字符

printable_character: 所有可打印ASCII字符

single_bracket_string_expression: "{" expression string_format? "}"

double_bracket_string_expression: "{{" expression string_format? "}}"

string_format: "!" string_format_prefix? string_format_width? string_format_precision? string_format_type

string_format_prefix: "0"
    | "-"

string_format_width: decimal_integer

string_format_precision: "." decimal_integer

string_format_type: "o"
    | "d"
    | "u"
    | "x"
    | "f"
    | "e"
    | "n"
    | "c"
    | "s"
```