# Basic Syntax
## Syntax Description
In OX syntax documents, syntax rules are represented by syntax descriptions. For example:
```
tvalue: "true"
```
The above is called a production rule of a non-terminal symbol. The left side of the colon is the name of the non-terminal symbol defined by the syntax, and the right side is the elements that make up this non-terminal symbol.
When the OX compiler detects that the elements on the right side appear in sequence in the source file, these elements are reduced to the non-terminal symbol on the left side.
For example, the above production rule means that when the string "true" appears in the source file, it is reduced to the tvalue non-terminal symbol.

A non-terminal symbol can sometimes be generated from multiple production rules, and we can separate them with "|" on the right side of the colon:
```
bool: "true"
    | "false"
```
The above rule means that when the string "true" or "false" appears in the source file, it can be reduced to the bool non-terminal symbol.

On the right side, "~" can be used to represent all characters in a range, for example:
```
digital: "0" ~ "9"
```
The above rule means that all characters from "0" to "9" can be reduced to the digital non-terminal symbol. It is equivalent to:
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
If an element may appear multiple times, the following repetition markers can be added to indicate the number of repetitions:

| Marker | Description |
|:-|:-|
|? | The element appears 0 or 1 time |
|+ | The element appears 1 or any number of times |
|* | The element appears 0 or any number of times |
|{N} | The element appears N times |

For example:
```
digital: "0" ~ "9"

integer: "-"? digital+
```
The rule for integer means that an integer may be preceded by a minus sign "-", followed by 1 or more numeric characters.

Parentheses can be used to group several elements, and a repetition marker can be added after the parentheses to indicate the number of repetitions of this group of data, for example:
```
digital: "0" ~ "9"

number_list: digital+ ("," digital+)*
```
## Identifiers
An identifier starts with an identifier initial character, followed by 0 or more identifier subsequent characters.

Identifier initial characters can be: "$", "_", "a" ~ "z", "A" ~ "Z"

Identifier subsequent characters can be: "$", "_", "a" ~ "z", "A" ~ "Z", "0" ~ "9"

The following are all valid identifiers:
```
a
a0
_my_var_0
$var0
```
Note that the following identifiers have special meanings in the OX language, so do not use them as variable names.

| Name | Description |
|:-|:-|
|null | null literal |
|true false | Boolean literals |
|this | The this parameter of the current function |
|argv | The parameter array of the current function |
|$ | Parameter 0 of the current function |
|$N | The Nth parameter of the current function. N is a decimal digit. |
|Identifier-type keywords | ref, if, for, do, while, try, catch, etc. |

Syntax description:
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

## Keywords
The following identifiers are used as keywords in the OX language, so do not use them as variable or constant names.
```
null true false
func class static enum bitfield if elif else while do for as
case break continue return try catch finally throw typeof instof
ref public this argv sched yield global owned textdomain
```