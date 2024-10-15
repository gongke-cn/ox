# Literals
Each literal in the OX language represents a value.
## null
"null" is a special literal indicating a null value. In the OX language, if a variable is not set, its default value is "null".

Syntax description:
```
null_literal: "null"
```
## Boolean Literals
The OX language includes two boolean literals, "true" and "false", corresponding to the boolean true and false respectively.

Syntax description:
```
bool_literal: "true"
    | "false"
```
## Numeric Literals
Numeric values in the OX language comply with the "IEEE754" double-precision number standard. A numeric value is stored in 64 bits.
Numeric literals in the OX language have the following forms:

Syntax description:
```
number_literal: decimal_integer_literal
    | binary_integer_literal
    | octal_integer_literal
    | hexadecimal_integer_literal
    | decimal_number_literal
    | exponential_number_literal
```
### Decimal Integers
Consist of a sequence of one or more characters from "0" to "9". For example:
```
0
1234567890
0123456789
```

Syntax description:
```
decimal_integer_literal: digital_character digital_character_or_separator*

digital_character_or_separator: digital_character
    | number_separator

digital_character: "0" ~ "9"

decimal_integer: digital_character+
```
### Binary Integers
Start with "0b" or "0B", followed by a sequence of one or more "0" or "1" characters. For example:
```
0b11110000
0B01010101
```

Syntax description:
```
binary_integer_literal: binary_prefix binary_character_or_separator+

binary_prefix: "0b"
    | "0B"

binary_character_or_separator: "0"
    | "1"
    | number_separator
```
### Octal Integers
Start with "0o" or "0O", followed by a sequence of one or more characters from "0" to "7". For example:
```
0o76543210
0O01234567
```

Syntax description:
```
octal_integer_literal: octal_prefix octal_character_or_separator+

octal_prefix: "0o"
    | "0O"

octal_character_or_separator: octal_character
    | number_separator

octal_character: "0" ~ "7"
```
### Hexadecimal Integers
Start with "0x" or "0X", followed by a sequence of one or more characters from "0" to "9", "a" to "f", or "A" to "F". For example:
```
0offee0123
0O0844AEFF
```

Syntax description:
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
### Decimal Number Form
Formatted as "integer part . fractional part", where both the integer and fractional parts are sequences of one or more "0" to "9" characters. For example:
```
3.1415926
0.0
0.0001234
```
Note that unlike the C language, neither the integer nor the fractional part can be empty. The following literal writings are incorrect:

```
.123 //Error: The integer part cannot be empty
123. //Error: The fractional part cannot be empty
```

Syntax description:
```
decimal_number_literal: digital_character digital_character_or_separator* "." digital_character_or_separator+
```
### Exponential Form
Add "e" or "E" and an exponent part after a decimal integer or decimal number form. The exponent part is a sequence of one or more "0" or "1" characters. A sign "+" or "-" can be added before the exponent part. For example:
```
1e10
1e+10
1E-10
12.345e5
```

Syntax description:
```
exponential_number_literal: ddigital_character digital_character_or_separator* ("." digital_character_or_separator+)? exponential_flag plus_or_minus? digital_character_or_separator+

exponential_flag: "e"
    | "E"

plus_or_minus: "+"
    | "-"
```
## NAN and Infinity
"Number.NAN" stands for "Not a number", meaning the stored value is not a valid "IEEE754" numeric value.
"Number.INFINITY" represents an infinite value.
### Numeric Separators
When writing a number with many digits, to avoid errors, you can insert "_" between numeric characters. The "_" does not affect the calculation of the numeric value. For example:
```
123_456_789
0xfe33_543f_92ab_ec33
```

Syntax description:
```
number_separator: "_"
```
## Character Literals
In the OX language, a character literal is represented by a single character enclosed in a pair of single quotes. For example:
```
'a'
'0'
'!'
```
The value of a character literal is equal to the ASCII code value of the character. For example:
```
'a' //equals 97
'0' //equals 48
'!' //equals 33
```
Escape characters with "\" can also be included in character literals:

|Character|Value|
|:-|:-|
|'\n'|LF Line Feed|
|'\r'|CR Carriage Return|
|'\t'|HT Horizontal Tab|
|'\v'|VT Vertical Tab|
|'\f'|FF Form Feed|
|'\a'|BEL Bell Character|
|'\b'|BS Backspace|
|'\\'|Backslash|
|'\''|Single Quote|
|'\"'|Double Quote|
|'\xNN'|Character with code value NN, where N is a hexadecimal character|
|'\uNNNN'|Unicode character with code value NNNN, where N is a hexadecimal character|
|'\u{N...}'|Unicode character with code value NNNN, where N is a hexadecimal character and there can be one or more Ns|

Syntax description:
```
character_literal: "'" character_data "'"

character_data: printable_character_except_single_quotation
    | escape_character

printable_character_except_single_quotation: All printable ASCII characters except "'" and "\"

escape_character: "\" "n"
    | "\" "r"
    | "\" "t"
    | "\" "v"
    | "\" "f"
    | "\" "a"
    | "\" "b"
    | "\" "\\"
    | "\" "'"
    | "\" "\""
    | "\" "x" hexadecimal_character{2}
    | "\" "u" hexadecimal_character{4}
    | "\" "u" "{" hexadecimal_character+ "}"
```
## String Literals
A string is a sequence of 0 to multiple 8-bit characters. In the OX language, strings are encoded in "UTF-8".
A string is enclosed in a pair of double quotes containing multiple characters, and escape characters with "\" can also be used in double quotes.
For example:
```
"" //Empty string
"a"
"hello!"
"line 1\nline2" //Contains escaped line break
```
Line breaks can also be directly added to a string to represent a multi-line string:
```
"line 1
line 2
line 3"
```
In the OX language, a string can also be represented by multiple characters enclosed in a pair of "''". This form is mainly used to better represent multi-line text. For example:
```
''string'' //Equivalent to "string"

''line 1
line 2
'' /*Equivalent to "line 1\nline 2"*/

''
line 1
line 2
line 3
'' /* Equivalent to "line 1\nline 2\nline 3"*/
```
Note that when using "''" to represent a multi-line string, if the line immediately after the starting "''" consists entirely of spaces, this line is ignored and the string is considered to start from the next line; if the line up to the ending "''" consists entirely of spaces, this line is also ignored.

Unlike double-quoted strings, escape characters are not supported in single-quoted strings. For example:
```
''\n\r\a\b'' //Equivalent to "\\n\\r\\a\\b"
```
To facilitate the construction of complex strings, the OX language allows embedding expressions in strings to dynamically build strings. For example:
```
name = "Bob"
text = "{name} is a good man."
```
In the following double-quoted string literal, the expression name is embedded in the string using a pair of curly braces "{" and "}", so that modifying the value of name replaces the corresponding string value of text.

In single-quoted strings, expressions are embedded using "{{" and "}}". For example:
```
''
{{name}} is my bro!
''
```
A format description can be added after the embedded expression to specify the format in which the expression is converted to a string.
```
"number: {n!08x}"
```
The expression and the format description are separated by the character "!". The format description consists of the following parts from left to right:

* Prefix (optional): Can be one of the following. If omitted, spaces are used for padding and right alignment is adopted.
    - 0: Automatically pad 0s before the numeric value for placeholder
    - -: Left alignment
* Width (optional): A decimal number indicating the total width of the string converted from the expression. If the actual width is less than this value, "0" (when the prefix is "0") or spaces are filled as placeholders.
* Precision (optional): A decimal number following ".", indicating the precision of floating-point numbers
* Format: Can be one of the following
    - o: The expression is an unsigned integer, output in octal
    - d: The expression is an integer, output in decimal
    - u: The expression is an unsigned integer, output in decimal
    - x: The expression is an unsigned integer, output in hexadecimal
    - f: The expression is a floating-point number, output in decimal number form
    - e：The expression is a floating-point number, output in exponential form
    - n：The expression is a numeric value, automatically output in decimal integer, decimal number or exponential form
    - c: The expression is a numeric value, treated as an ASCII code value to output the corresponding character
    - s: The expression is a string

Syntax description:
```
string_literal: "\"" double_quotation_string_item* "\""
    | "''" single_quotation_string_item* "''"

double_quotation_string_item: printable_character_except_double_quotation
    |escape_character
    |single_bracket_string_expression

single_quotation_string_item: printable_character
    |double_bracket_string_expression

printable_character_except_double_quotation: All printable ASCII characters except "\""

printable_character: All printable ASCII characters

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