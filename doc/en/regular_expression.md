# Regular Expressions
## Overview
Regular expressions are used for string matching. See the example below:
```
/[a-z]+/i
```
The above statement declares a regular expression. Between a pair of "/" symbols is the definition of the matching pattern of the regular expression.
After the latter "/", are the attribute flags of the regular expression. Here, the attribute flag "i" means case-insensitive.
We can use this regular expression to match strings:
```
“123 Hello ?” ~ /[a-z]+/i //returns "Hello"
“123 Hello ?”.match(/[a-z]+/i) //returns a Match object
“123?” ~ /[a-z]+/i //returns null
```

Syntax Description:
```
regular_expression: "/" regular_expression_conditions "/" regular_expression_flag*

regular_expression_pattern: regular_expression_conditions

regular_expression_conditions: regular_expression_conditions "|" regular_expression_condition
    | regular_expression_condition

regular_expression_condition: regular_expression_item*

regular_expression_item: regular_expression_match regular_expression_times? "?"?
    | "^"
    | "$"

regular_expression_match: "."
    | regular_expression_character
    | "\s"
    | "\S"
    | "\d"
    | "\D"
    | "\w"
    | "\W"
    | "\b"
    | "\B"
    | "[" regular_expression_class_item* "]"
    | "[^" regular_expression_class_item* "]"
    | "(" regular_expression_pattern ")"
    | "(:" regular_expression_pattern ")"
    | "(?=" regular_expression_pattern ")"
    | "(?!" regular_expression_pattern ")"
    | "(?<=" regular_expression_pattern ")"
    | "(?<!" regular_expression_pattern ")"
    | "\" decimal_integer

regular_expression_character: All printable ASCII characters except "/", "\", "|", "(", ")", "[", "]", ".", "^", "$"

regular_expression_class_item: regular_expression_class_character
    | regular_expression_class_character "-" regular_expression_class_character
    | "\s"
    | "\S"
    | "\d"
    | "\D"
    | "\w"
    | "\W"

regular_expression_class_character: All printable ASCII characters except "]", "-"

regular_expression_times: "?"
    | "+"
    | "*"
    | "{" decimal_integer "}"
    | "{" decimal_integer "," "}"
    | "{" decimal_integer "," decimal_integer "}"

regular_expression_flag: "i"
    | "m"
    | "a"
    | "u"
    | "p"
```
## Match Object
When a string calls the "match" attribute function, it returns a Match object if the match is successful. The Match object contains the following attributes:

|Attribute|Type|Description|
|:-|:-|:-|
|$to_str()|Return value is a string|Returns the matched string|
|start|Positive integer|The starting position of the matched string in the original string (the index value of the first character)|
|end|Positive integer|The ending position of the matched string in the original string (the index value of the last character + 1)|
|groups|String array|Array element 0 is the matched string, and the remaining elements correspond to the matched substrings of the matching subpatterns|
|slices|[start, end] position array|Each element is a 2-element array, corresponding to the start and end positions of each matched string|

## Syntax
### Attribute Flags
|Attribute|Description|
|:-|:-|
|i|Case-insensitive|
|m|Multi-line matching, the line end "$" can match LF line breaks; without this attribute, "$" can only match the end of the string|
|a|"." can match any character (including LF line breaks); without this attribute, "." cannot match LF|
|u|Unicode matching; without this attribute, matching is performed according to ASCII|
|p|Exact string matching; without this attribute, the regular expression can match a substring of the string|

### Any Character Matching
"." is used to match any character:
```
"a" ~ /./ //returns "a"
"1" ~ /./ //returns "1"
" " ~ /./ //returns " "
"" ~ /./ //The string is empty, cannot match ".", returns null
```
Note that if the regular expression does not have the "a" attribute, "." cannot match LF line breaks:
```
"\n" ~ /./ //returns null
"\n" ~ /./a //returns "\n"
```
### Character Matching
Printable ASCII characters, except for special command characters "/", "\", "|", "(", ")", "[", "]", ".", "^", "$", can be used to match a single character.
```
/a/ //matches the character "a"
/A/ //matches the character "A"
/0/ //matches the character "0"
/!/ //matches the character "!"
/ / //matches the space character " "
```
Escaped characters "\" can be used to match some special characters:
```
/\n/ //matches LF line break
/\r/ //matches CR carriage return
/\t/ //matches HT horizontal tab
/\v/ //matches VT vertical tab
/\f/ //matches FF (form feed)
/\a/ //matches BEL (bell)
/\b/ //matches BS (backspace)
/\// //matches "/"
/\\/ //matches "\"
/\|/ //matches "|"
/\x41/ //matches the character with ASCII code 0x41 ("A")
/\u0041/ //matches the character with Unicode code 0x0041 ("A")
/\u{41}/ //matches the character with Unicode code 0x41 ("A")
```
Multiple characters are used to match a character sequence:
```
/abc/ //matches the substring "abc"
/123\|/ //matches the substring "123|"
```
### Type Character Matching
Type character matching can be used to match a class of special characters, including:

|Type|Description|
|:-|:-|
|\s|Whitespace characters, including " ", "\t", "\v", "\f", "\n", "\r"|
|\S|Non-whitespace characters, characters other than " ", "\t", "\v", "\f", "\n", "\r"|
|\d|Digit characters, including "0" ~ "9"|
|\D|Non-digit characters, characters other than "0" ~ "9"|
|\w|Word characters, including "0" ~ "9", "a" ~ "z", "A" ~ "Z", "_"|
|\W|Non-word characters, characters other than "0" ~ "9", "a" ~ "z", "A" ~ "Z", "_"|

Examples:
```
"A " ~ /\s/ //returns " "
"A\t" ~ /\s/ //returns "\t"
" \tA" ~ /\S/ //returns "A"
"0" ~ /\d/ //returns "0"
"9" ~ /\d/ //returns "9"
"0123456789A" ~ /\D/ //returns "A"
" A" ~ /\w/ //returns "A"
" 0" ~ /\w/ //returns "0"
" _" ~ /\w/ //returns "_"
" a" ~ /\W/ //returns " "
```

### Line Start Matching
"^" can be used to match the start of a line:
```
"abc" ~ /^abc/ //returns "abc"
"123abc" ~ /^abc/ //"abc" does not appear at the start position, returns null
```
Note that if the regular expression does not have the "m" attribute, "^" can only match the start of the string; when the regular expression has the "m" attribute, "^" can match the start position of all lines (including after LF line breaks):
```
"123\nabc" ~ /^abc/ //returns null
"123\nabc" ~ /^abc/m //returns "abc"
```
### Line End Matching
"$" can be used to match the end of a line:
```
"abc" ~ /abc$/ //returns "abc"
"abc123" ~ /abc$/ //"abc" does not appear at the end position, returns null
```
Note that if the regular expression does not have the "m" attribute, "$" can only match the end of the string; when the regular expression has the "m" attribute, "$" can match the end position of all lines (including before LF line breaks):
```
"abc\n123" ~ /abc$/ //returns null
"abc\n123" ~ /abc$/m //returns "abc"
```
### Word Boundary Matching
"\b" can be used to match a word boundary position, i.e., the two characters before and after this position are a word character and another (non-word character or line start/end) respectively.
Examples:
```
"12abc34" ~ /\babc\b/ //The two sides of "abc" are not boundaries, so returns null
"123 abc 456" ~ /\babc\b/ //returns "abc"
"123 abc" ~ /\babc\b/ //returns "abc"
```
"\B" means matching a non-word boundary position, e.g.:
```
"12abc34" ~ /\Babc\B/ //returns "abc"
"123 abc 456" ~ /\Babc\B/ //returns null
"123 abc" ~ /\Babc\B/ //returns null
```
### Character Set Matching
A pair of "[" and "]" commands represent a character set, used to match any one character in this set:
```
/[abc]/ //can match any one of the characters "a", "b", "c"
/[a-z]/ //can match any lowercase letter
/[\s0-9,]/ //can match all whitespace characters, "0" ~ "9", and the character ","
```
Adding the character "^" at the front of the character set means inverse matching, i.e., the match is successful when the character is not in this set:
```
/[^a-zA-Z]/ //matches non-alphabetic characters
/[^0-9]/ //matches non-digit characters
```
### Match Count Flags
A match count flag can be added after each matching command to indicate the number of matches required:

|Flag|Description|
|:-|:-|
|?|Matches 0 or 1 time|
|+|Matches 1 or any number of times|
|*|Matches 0 or any number of times|
|{N}|Matches N times|
|{N,M}|Matches at least N times and at most M times|
|{N,}|Matches at least N times and at most any number of times|

Examples:
```
"" ~ /a?/ //Matches 0 times, returns ""
"aa" ~ /a?/ //Matches 1 time, returns "a"
"" ~ /a+/ //returns null
"a" ~ /a+/ //Matches 1 time, returns "a"
"aa" ~ /a+/ //Matches 2 times, returns "aa"
"aaa" ~ /a+/ //Matches 3 times, returns "aaa"
"" ~ /a*/ //Matches 0 times, returns ""
"a" ~ /a*/ //Matches 1 time, returns "a"
"aa" ~ /a*/ //Matches 2 times, returns "aa"
"aaa" ~ /a*/ //Matches 3 times, returns "aaa"
"a" ~ /a{2,3}/ //returns null
"aa" ~ /a{2,3}/ //returns "aa"
"aaaa" ~ /a{2,3}/ //Matches at most 3 times, returns "aaa"
```
### Greedy Mode and Non-Greedy Mode
Consider the following regular expression:
```
/[a-z]+a/
```
Using this expression to match the string "abcaba": the first 3 characters "abc" successfully match "[a-z]+". When starting to match the fourth character "a", we can continue to match "[a-z]+", or match the subsequent command "a".
We call the former way of prioritizing multiple matches greedy mode, and the latter way of prioritizing subsequent commands non-greedy mode.

The "OX" regular expression uses greedy mode by default. Adding "?" after a matching command means using non-greedy mode:
```
"abcaba" ~ /[a-z]+a/ //Greedy mode, returns "abcaba"
"abcaba" ~ /[a-z]+?a/ //Non-greedy mode, returns "abca"
```
### Lookahead Matching
Sometimes we want the match to occur only before certain specific matches are successful. In this case, we can use lookahead matching:
```
/[a-z]+(?=[0-9])/
```
The lookahead matching "[0-9]" is defined between "(?=" and ")". Only when the lookahead matching is successful, the "[a-z]+" match can be successful. So:
```
"abcd" ~ /[a-z]+(?=[0-9])/ //Does not meet the lookahead condition, returns null
"abc0" ~ /[a-z]+(?=[0-9])/ //Meets the lookahead condition, returns "abc"
```
In addition, "(?!" and ")" can be used to indicate negative lookahead, i.e., the entire match is successful only when the lookahead is not satisfied:
```
"abcd" ~ /[a-z]+(?![0-9])/ //Does not meet the lookahead condition, returns "abcd"
"abc0" ~ /[a-z]+(?![0-9])/ //Meets the lookahead condition, returns null
```
### Lookbehind Matching
Similar to lookahead matching, we can also use lookbehind matching to define that the entire match is successful only after a specific match occurs:
```
/(?<=[0-9])[a-z]+/
```
The lookbehind matching pattern is defined between "(?<=" and ")".
```
"abc" ~ /(?<=[0-9])[a-z]+/ //Does not meet the lookbehind condition, returns null
"zabc" ~ /(?<=[0-9])[a-z]+/ //Does not meet the lookbehind condition, returns null
"0abc" ~ /(?<=[0-9])[a-z]+/ //Meets the lookbehind condition, returns "abc"
```
Similar to lookahead matching, "(?<!" and ")" can be used to define negative lookbehind. The entire match is successful only when the negative lookbehind does not match:
```
"abc" ~ /(?<![0-9])[a-z]+/ //Does not meet the lookbehind condition, returns "abc"
"zabc" ~ /(?<![0-9])[a-z]+/ //Does not meet the lookbehind condition, returns "zabc"
"0abc" ~ /(?<![0-9])[a-z]+/ //Meets the lookbehind condition, returns null
```
### Subpatterns
When a matching pattern is complex, some subpatterns can be packaged through "(:" and ")".
```
"var=1978" ~ /(:[a-z]+)=(:[0-9]+)/ //returns "var=1978"
```
Two subpatterns "(:[a-z]+)" and "(:[0-9]+)" are defined in the regular expression.
### Matching Subpatterns
Sometimes during matching, we not only need to get the complete matched string, but also want to get the strings corresponding to each subpattern. We can define matching subpatterns to get these substrings.
In a regular expression, a matching subpattern can be defined through a pair of parentheses "(" and ")":
For example:
```
m = "var=1978".match(/([a-z]+)=([0-9]+)/)
stdout.puts("match: {m}\n") //prints "var=1978"
stdout.puts("start: {m.start} end: {m.end}\n") //prints the start and end positions of the match: 0 and 8
stdout.puts("name: {m.groups[1]}\n") //prints the string matched by the first matching subpattern: "var"
stdout.puts("name start: {m.slices[1][0]} end: {m.slices[1][1]}\n") //prints the start and end positions of the match of the first matching subpattern: 0 and 3
stdout.puts("value: {m.groups[2]}\n") //prints the string matched by the second matching subpattern: "1978"
stdout.puts("value start: {m.slices[2][0]} end: {m.slices[2][1]}\n") //prints the start and end positions of the match of the first matching subpattern: 4 and 8
```
### Backreference Matching
In matching commands, sometimes we need to reference a previously matched string. In this case, we can use backreference matching.
A decimal number following a backslash "\" indicates the sequence number of the matching subpattern (starting from 1).
For example:
```
/([a-z]+)=\1_value/
```
"\1" in the regular expression means referencing the string matched by matching subpattern 1:
```
"abc=abc_value" ~ /([a-z]+)=\1_value/ //returns "abc=abc_value"
"def=def_value" ~ /([a-z]+)=\1_value/ //returns "def=def_value"
```
### Multiple Option Matching
Multiple matching options can be listed through "|":
```
/LiuBang|XiangYu|ZhangHan/
```
There are three matching options in the regular expression. During matching, each option is tried from left to right. If one option matches successfully, the matching result is returned; otherwise, the next option is tried. If all options fail, the match is considered failed.
```
"LiuBang" ~ /LiuBang|XiangYu|ZhangHan/ //returns "LiuBang"
"XiangYu" ~ /LiuBang|XiangYu|ZhangHan/ //returns "XiangYu"
"ZhangHan" ~ /LiuBang|XiangYu|ZhangHan/ //returns "ZhangHan"
"ZhaoGao" ~ /LiuBang|XiangYu|ZhangHan/ //returns null
```