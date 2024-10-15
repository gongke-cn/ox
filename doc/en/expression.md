# Expressions
Syntax Description:
```
expression: variable_declaration
    | constant_declaration
    | null_literal
    | bool_literal
    | number_literal
    | character_literal
    | string_literal
    | array_literal
    | array_append_items
    | object_literal
    | object_append_properties
    | parentheses_expression
    | get_property_expression
    | get_item_expression
    | function_call_expression
    | unary_expression
    | math_expression
    | shift_expression
    | bitwise_expression
    | instof_expression
    | match_expression
    | compare_expression
    | equal_expression
    | logic_and_expression
    | logic_or_expression
    | assignment_expression
    | reverse_assignment_expression
    | self_operation_assignment_expression
    | regular_expression
    | if_statement
    | case_statement
    | function_declaration
    | class_declaration
    | yield_expression
    | get_pointer_expression
    | get_value_expression
```
## Operator Precedence
|Precedence|Operator|Description|Associativity|
|:-|:-|:-|:-|
|1|()|Parentheses|Left to right|
|2|.|Access property|Left to right|
|2|[]|Access element|Left to right|
|2|()|Call function|Left to right|
|3|-|Unary minus|Right to left|
|3|+|Unary plus|Right to left|
|3|!|Logical NOT operation|Right to left|
|3|~|Bitwise NOT operation|Right to left|
|3|*|C pointer dereference operation|Right to left|
|3|&|C value address-of operation|Right to left|
|3|typeof|Get object class|Right to left|
|4|**|Exponentiation|Left to right|
|5|*|Multiplication|Left to right|
|5|/|Division|Left to right|
|5|%|Modulus|Left to right|
|6|+|Addition|Left to right|
|6|-|Subtraction|Left to right|
|7|<<|Left shift|Left to right|
|7|>>|Right shift (highest bit not involved in shifting)|Left to right|
|7|>>>|Unsigned right shift (highest bit involved in shifting)|Left to right|
|8|<|Less than|Left to right|
|8|<=|Less than or equal to|Left to right|
|8|>|Greater than|Left to right|
|8|>=|Greater than or equal to|Left to right|
|8|<|Less than|Left to right|
|8|instof|Instance check|Left to right|
|9|==|Equal to|Left to right|
|9|!=|Not equal to|Left to right|
|10|&|Bitwise AND|Left to right|
|11|^|Bitwise XOR|Left to right|
|12|\||Bitwise OR|Left to right|
|13|&&|Logical AND|Left to right|
|14|\|\||Logical OR|Left to right|
|15|=|Assignment|Right to left|
|15|+=|Add and assign|Right to left|
|15|-=|Subtract and assign|Right to left|
|15|*=|Multiply and assign|Right to left|
|15|/=|Divide and assign|Right to left|
|15|%=|Modulus and assign|Right to left|
|15|**=|Exponentiate and assign|Right to left|
|15|<<=|Left shift and assign|Right to left|
|15|>>=|Right shift and assign|Right to left|
|15|>>>=|Unsigned right shift and assign|Right to left|
|15|&=|Bitwise AND and assign|Right to left|
|15|\|=|Bitwise OR and assign|Right to left|
|15|^=|Bitwise XOR and assign|Right to left|
|15|&&=|Assign if true|Right to left|
|15|\|\|=|Assign if false|Right to left|

## Type Conversion Operations
During some operations, OX automatically converts operands to different types. Common type conversions include the following.
### Convert to Boolean
|Original Type|Result|
|:-|:-|
|null|false|
|Boolean|Returns the original value|
|Number|false if the number is 0, true for other values|
|String|false if the length is 0, true for other values|
|Object|true|

### Convert to Number
|Original Type|Result|
|:-|:-|
|null|0|
|Boolean|1 for true, 0 for false|
|Number|Returns the original value|
|String|Parsed as a numeric literal, returns the resulting number. If the string cannot be parsed as a number, returns Number.NAN|
|Object|Calls the object's "$to_num()" method and returns the resulting number|

### Convert to 32-bit Unsigned Integer
The specific operations are as follows:

* Convert the operand to a number
* If the number is less than 0 or greater than 4294967295, throw a RangeError
* Truncate to the integer part to become an integer

### Convert to 32-bit Signed Integer
The specific operations are as follows:

* Convert the operand to a number
* If the number is less than -2147483648 or greater than 2147483647, throw a RangeError
* Truncate to the integer part to become an integer

### Convert to 64-bit Unsigned Integer
The specific operations are as follows:

* If the operand is a 64-bit integer, directly convert it to an unsigned 64-bit integer and return
* Convert the operand to a number
* If the number is less than 0 or greater than 18446744073709551615, throw a RangeError
* Truncate to the integer part to become an integer

### Convert to 64-bit Signed Integer
The specific operations are as follows:

* If the operand is a 64-bit integer, directly convert it to a signed 64-bit integer and return
* Convert the operand to a number
* If the number is less than -9223372036854775808 or greater than 9223372036854775807, throw a RangeError
* Truncate to the integer part to become an integer

### Convert to String
|Original Type|Result|
|:-|:-|
|null|""|
|Boolean|"true" for true, "false" for false|
|Number|Convert the number to a string in the format "n"|
|String|Returns the original value|
|Object|Calls the object's "$to_str()" method and returns the resulting string|

## Parentheses
Parentheses have the highest precedence in all operations.
```
1 + 2 * 3 //Multiplication has higher precedence than addition, result is 7
(1 + 2) * 3 //Parentheses have the highest precedence, result is 9
```

Syntax Description:
```
parentheses_expression: "(" expression ")"
```
## Access Property
Access a property of an object.
```
o.prop //Access the property "prop" of object o
"string".length //Access the property "length" of the string "string"
a.b.c //Get the object of property "b" of object a, and access the property "c" of this object
```
The expression before the "." symbol is an object expression, and the identifier after the "." symbol represents the name of the property.

Syntax Description:
```
get_property_expression: expression "." identifier
```
## Access Element
Access an element of an array or a property of an object.
```
a[0] //Access element 0 of array a
o["prop"] //Access the property "prop" of object o
```
The part before the square brackets is the array or object to be accessed, and the part inside the square brackets is a valid expression. If the expression type is a number, it represents the index of the element to be accessed. If the expression type is a string, it represents the name of the property to be accessed.

Syntax Description:
```
get_item_expression: expression "[" expression "]"
```
## Function Call
Call a function.
```
my_func() //Call the function "my_func"
my_func_1(0) //Call the function "my_func_1" and pass one parameter
my_func_2(0, 1) //Call the function "my_func_2" and pass two parameters
my_func_3(0, 1, my_func_4()) //Call the function "my_func_3" and pass three parameters, where the third parameter is the return value of calling the function "my_func_4"
```
If we want to pass multiple elements of an array as parameters to a function, we can use "..." to expand the array elements:
```
a = [1,2,3]
my_func(...a) //Equivalent to my_func(1,2,3)
a1 = [1,2,3]
a2 = [4,5,6]
my_func(...a1, ...a2) //Equivalent to my_func(1,2,3,4,5,6)
```

Syntax Description:
```
function_call_expression: expression "(" arguments? ")"

arguments: arguments "," argument
    | argument

argument: expression
    | "..." expression
```
## Unary Operation
A unary operation has one operand on the right side of the operator.

Syntax Description:
```
unary_expression: unary_operator expression

unary_operator: "+"
    | "-"
    | "!"
    | "~"
    | "typeof"
```
### +
The specific operation of the operation is as follows:

* If the operand is a 64-bit integer
    - Return a 64-bit integer
* If the operand is of other types
    - Convert the operand to a number
    - Return the number

```
+123
+1.234
+true //Result is 1
+“0” //Result is 0
```
### -
The specific operation of the operation is as follows:

* If the operand is a 64-bit integer
    - Convert the operand to a 64-bit signed integer
    - Negate the 64-bit signed integer
    - Return the resulting 64-bit signed integer
* If the operand is of other types
    - Convert the operand to a number
    - Negate the number
    - Return the resulting number

```
-1
-0.123
-"-3.14" //Result is 3.14
```
### Logical NOT
The specific operation of the operation is as follows:

* Convert the operand to a boolean value
* Negate the boolean value
* Return the resulting boolean value

```
!true //Result is false
!false //Result is true
!0 //Result is true
```
### Bitwise NOT
The specific operation of the operation is as follows:

* If the operand is a 64-bit integer
    - Convert the operand to an unsigned 64-bit integer
    - Perform a bitwise NOT operation on the unsigned 64-bit integer
    - Return the resulting unsigned 64-bit integer
* If the operand is not a 64-bit integer
    - Convert it to an unsigned 32-bit integer
    - Perform a bitwise NOT operation on the unsigned 32-bit integer
    - Convert the result to a number
    - Return the resulting number

```
~0 //Result is 0xffffffff
~1 //Result is 0xfffffffe
```
### typeof
Returns the class to which the operand belongs:

|Operand Type|typeof Result|
|:-|:-|
|null|null|
|Boolean|Bool|
|Number|Number|
|String|String|
|Object|The class to which the object belongs|
|C Value|The C type corresponding to the C value|
|Others|null|

## Mathematical Operations
Mathematical operations are binary operations.

Syntax Description:
```
math_expression: expression math_operator expression

math_operator: "+"
    | "-"
    | "*"
    | "/"
    | "%"
    | "**"
```
### Addition
The specific operation of the operation is as follows:

* If one of the two operands is a string
    - Convert both operands to strings
    - Perform string concatenation
    - Return the resulting string
* If one of the two operands is a 64-bit integer
    - If one of the two operands is an unsigned 64-bit integer
        * Convert both operands to unsigned 64-bit integers
        * Perform 64-bit integer addition
        * Return the resulting unsigned 64-bit integer
    - If neither of the two operands is an unsigned 64-bit integer
        * Convert both operands to signed 64-bit integers
        * Perform 64-bit integer addition
        * Return the resulting signed 64-bit integer
* Other operand types
    - Convert both operands to numbers
    - Perform addition
    - Return the resulting number

```
1 + 2 //Result is 3
"a" + "b" //Result is "ab"
```
### Subtraction/Multiplication/Division/Modulus/Exponentiation

|Operator|Description|
|:-|:-|
|-|Subtraction|
|*|Multiplication|
|/|Division|
|%|Modulus|
|**|Exponentiation|

The specific operation of the operation is as follows:

* If one of the two operands is a 64-bit integer
    - If one of the two operands is an unsigned 64-bit integer
        * Convert both operands to unsigned 64-bit integers
        * Perform 64-bit unsigned integer mathematical operation
        * Return the resulting unsigned 64-bit integer
    - If neither of the two operands is an unsigned 64-bit integer
        * Convert both operands to signed 64-bit integers
        * Perform 64-bit signed integer mathematical operation
        * Return the resulting signed 64-bit integer
* Other operand types
    - Convert both operands to numbers
    - Perform mathematical operation
    - Return the resulting number

```
1 - 2 //Result is -1
2 * 3 //Result is 6
3 / 2 //Result is 1.5
3 % 2 //Result is 1
```

## Shift Operations

|Operator|Description|
|:-|:-|
|<<|Left shift|
|>>|Right shift (highest bit not involved in shifting)|
|>>>|Unsigned right shift (highest bit involved in shifting)|

The specific operation of the operation is as follows:

* If the left operand is a 64-bit integer
    - If it is an unsigned right shift (>>>) operation
        * Convert the left operand to an unsigned 64-bit integer
        * Convert the left operand to an unsigned 32-bit integer
        * Perform unsigned right shift operation
        * Return the resulting unsigned 64-bit integer
    - If it is not an unsigned right shift (>>>) operation
        * Convert the left operand to a signed 64-bit integer
        * Convert the left operand to an unsigned 32-bit integer
        * Perform signed shift operation (the highest bit is regarded as the sign bit and does not move during right shift)
        * Return the resulting signed 64-bit integer
* Other operand types
    - If it is an unsigned right shift (>>>) operation
        * Convert the left operand to an unsigned 32-bit integer
        * Convert the left operand to an unsigned 32-bit integer
        * Perform unsigned right shift operation
        * Return the resulting unsigned 32-bit integer
    - If it is not an unsigned right shift (>>>) operation
        * Convert the left operand to a signed 32-bit integer
        * Convert the left operand to an unsigned 32-bit integer
        * Perform signed shift operation (the highest bit is regarded as the sign bit and does not move during right shift)
        * Return the resulting signed 32-bit integer

```
1 << 1 //Left shift, result is 2
2 >> 1 //Right shift, result is 1
-2 >> 1 //Right shift, result is -1
-2 >>> 1 //Unsigned right shift, result is 0x7fffffff
```

Syntax Description:
```
shift_expression: expression shift_operator expression

shift_operator: "<<"
    | ">>"
    | ">>>"
```
## Bitwise Operations

|Operator|Description|
|:-|:-|
|\||Bitwise OR|
|&|Bitwise AND|
|^|Bitwise XOR|

The specific operation of the operation is as follows:

* If one of the two operands is a 64-bit integer
    - If one of the two operands is an unsigned 64-bit integer
        * Convert both operands to unsigned 64-bit integers
        * Perform 64-bit integer bitwise operation
        * Return the resulting unsigned 64-bit integer
    - If neither of the two operands is an unsigned 64-bit integer
        * Convert both operands to signed 64-bit integers
        * Perform 64-bit integer bitwise operation
        * Return the resulting signed 64-bit integer
* Other operand types
    - Convert both operands to unsigned 32-bit integers
    - Perform bitwise operation
    - Convert the resulting unsigned 32-bit integer to a number
    - Return the number

```
0b11110000 | 0b00001111 //Bitwise OR, result is 0b1111_1111
0b11111111 & 0b11110000 //Bitwise AND, result is 0b1111_0000
0b11110000 ^ 0b11000011 //Bitwise XOR, result is 0b0011_0011
```

Syntax Description:
```
bitwise_expression: expression bitwise_operator expression

bitwise_operator: "|"
    | "&"
    | "^"
```

## instof
Check if the left operand is an instance of the right operand.
The right operand is generally a class. If the left operand is an instance of this class, the operation returns true; otherwise, it returns false.
```
true instof Bool //Returns true
1 instof Bool //Returns false
"str" instof String //Returns true
```

Syntax Description:
```
instof_expression: expression "instof" expression
```
## Match Operation
Match a string with a regular expression or a string. If the match is successful, return the matched string; otherwise, return null.
```
"abc123" ~ /[a-z]+/ //Returns "abc"
"abc123" ~ "123" //Returns "123"
"abc123" ~ "abcd" //Returns null
```

Syntax Description:
```
match_expression: expression "~" expression
```
## Comparison Operations

|Operator|Description|
|:-|:-|
|<|Less than|
|>|Greater than|
|<=|Less than or equal to|
|>=|Greater than or equal to|

The specific operation of the operation is as follows:

* If one of the two operands is a string
    - Convert both operands to strings
    - Perform string comparison
    - Return a boolean value indicating the comparison result
* If one of the two operands is a 64-bit integer
    - If one of the two operands is an unsigned 64-bit integer
        * Convert both operands to unsigned 64-bit integers
        * Compare the two unsigned 64-bit integers
        * Return a boolean value indicating the comparison result
    - If neither of the two operands is an unsigned 64-bit integer
        * Convert both operands to signed 64-bit integers
        * Compare the two signed 64-bit integers
        * Return a boolean value indicating the comparison result
* Other operand types
    - Convert both operands to numbers
    - Compare the two numbers
    - Return a boolean value indicating the comparison result

```
1 < 0 //Result is false
1 > 0 //Result is true
“a” < "b" //Result is true
```
String comparison operation is as follows:

* Compare characters in the two strings one by one starting from index 0
* If the characters are the same, compare the next character
* If the encoding value of the character in the left operand is less than that of the character in the right operand, end the comparison process, and the result is that the left string is less than the right string
* If the encoding value of the character in the left operand is greater than that of the character in the right operand, end the comparison process, and the result is that the left string is greater than the right string
* If the preceding characters of the two strings are the same, and the right string is longer than the left string, the result is that the left string is less than the right string
* If the preceding characters of the two strings are the same, and the right string is shorter than the left string, the result is that the left string is greater than the right string
* If the preceding characters of the two strings are the same, and the lengths of the left and right strings are also the same, the result is that the two strings are equal

When multiple comparison operations need to be performed on an expression:
```
0 < a && a < 10
```
It can be simplified to:
```
0 < a < 10
```

Syntax Description:
```
compare_expression: expression compare_operator expression

compare_operator: "<"
    | ">"
    | "<="
    | ">="
```

## Equality/Inequality Operations

|Operator|Description|
|:-|:-|
|==|Equal to|
|!=|Not equal to|

Compare whether two operands are equal.

* If both operands are strings
    - Perform string comparison
    - Return a boolean value indicating the comparison result
* If both operands are numbers or C values
    - Convert both numbers to the same type
    - Perform numeric comparison
    - Return a boolean value indicating the comparison result
* Operands of other types
    - If the two operands have the same type and value, they are considered equal; otherwise, they are not equal
    - Return a boolean value indicating the comparison result

```
1 == 1 //Result is true
1 != 0 //Result is true
0 == null //Result is false
```
Note that operand types are not automatically converted during equality/inequality comparison, so "0 == null" results in false.

Syntax Description:
```
equal_expression: expression equal_operator expression

equal_operator: "=="
    | "!="
```

## Logical AND Operation
The specific operation of the operation is as follows:

* Convert the left operand to a boolean value. If the result is false, directly return the original value of the left operand.
* Return the value of the right operand.

```
1 && 2 //Result is 2
1 && 0 //Result is 0
0 && 2 //Result is 0
```
Note that if the left operand is converted to a boolean value and the result is false, the right operand will not be executed.
```
false && run() //The run() function will not be called.
```

Syntax Description:
```
logic_and_expression: expression "&&" expression
```
## Logical OR Operation
The specific operation of the operation is as follows:

* Convert the left operand to a boolean value. If the result is true, directly return the original value of the left operand.
* Return the value of the right operand.

```
1 || 2 //Result is 1
0 || 1 //Result is 1
0 || 2 //Result is 2
```
Note that if the left operand is converted to a boolean value and the result is true, the right operand will not be executed.
```
true || run() //The run() function will not be called.
```

Syntax Description:
```
logic_or_expression: expression "||" expression
```
## Assignment
Assign the calculation result of an expression to an lvalue expression.
Note that only lvalue expressions can be assigned. Assigning to a non-lvalue expression will cause a compiler error.
Lvalue expressions can be the following types:

|Type|Example|
|:-|:-|
|Variable|v|
|Property|o.prop|
|Element|a[0]|
|Pointer dereference|*ptr|

See the following assignment statements:
```
v = 1 //Assign 1 to variable v
o.prop = true //Assign true to the property "prop" of object o
a[1] = "item 1" //Assign the string "item 1" to element 1 of array a
*ptr = 1 //Set the C value pointed to by the pointer to 1
```
The following assignment statements are incorrect:
```
c: 1 //Declare constant c
c = 2 //Assignment error: cannot assign to a constant.
100 = 1 //Assignment error: 100 is not an lvalue expression.
a + b = true //Assignment error: a + b is not an lvalue expression.
```

Syntax Description:
```
assignment_expression: assignment_left_value "=" expression

assignment_left_value: identifier
    | get_property_expression
    | get_item_expression
    | get_value_expression
```
## Reverse Assignment
OX language supports reverse assignment, in the form of:
```
1 => a
```
Reverse assignment uses the operator "=>". The right side of the operator is the lvalue expression of the assignment target, and the left side of the operator is the value to be set.
In addition to general assignment operations, reverse assignment statements also support assigning to multiple targets in one statement. For example, the following statement:
```
a => [v1, v2, v3]
```
It means to assign the values of elements 0, 1, and 2 of array a to variables v1, v2, and v3 respectively. This operation is equivalent to:
```
v1 = a[0]
v2 = a[1]
v3 = a[2]
```
Reverse assignment also supports assigning to multiple targets by matching object property names. For example, the following statement:
```
o => {p1:v1, p2:v2, p3:v3}
```
This operation is equivalent to:
```
v1 = o.p1
v2 = o.p2
v3 = o.p3
```
If the target variable name is the same as the property name, the assignment statement can be simplified to:
```
o => {p1, p2, p3}
```
This operation is equivalent to:
```
p1 = o.p1
p2 = o.p2
p3 = o.p3
```

Syntax Description:
```
reverse_assignment_expression: expression "=>" reverse_assginment_left_value

reverse_assginment_left_value: assginment_left_value
    | array_assignment_left_value
    | object_assignment_left_value

array_assignment_left_value: "[" array_assignment_left_items? "]"

array_assignment_left_items: array_assignment_left_items "," array_assignment_left_item
    | array_assignment_left_item

array_assignment_left_item: reverse_assginment_left_value

object_assignment_left_value: "{" object_assignment_left_items? "}"

object_assignment_left_items: object_assignment_left_items "," object_assignment_left_item
    | object_assignment_left_item

object_assignment_left_item: property_name ":" reverse_assginment_left_value
    | identifier
```
## Self-operation Assignment
Self-operation means first taking the value of the lvalue expression, then performing a calculation on the result, and then assigning the final calculation result to the lvalue expression.
```
a += 1 //Equivalent to a = a + 1
```
Self-operation operations include the following types:

|Operator|Description|
|:-|:-|
|+=|Add the lvalue and rvalue, then assign to the lvalue|
|-=|Subtract the rvalue from the lvalue, then assign to the lvalue|
|*=|Multiply the lvalue and rvalue, then assign to the lvalue|
|/=|Divide the lvalue by the rvalue, then assign to the lvalue|
|%=|Take the modulus of the lvalue with the rvalue, then assign to the lvalue|
|**=|Exponentiate the lvalue with the rvalue, then assign to the lvalue|
|\|=|Perform bitwise OR on the lvalue and rvalue, then assign to the lvalue|
|&=|Perform bitwise AND on the lvalue and rvalue, then assign to the lvalue|
|^=|Perform bitwise XOR on the lvalue and rvalue, then assign to the lvalue|
|<<=|Left shift the lvalue, then assign to the lvalue|
|>>=|Right shift the lvalue (highest bit not involved in shifting), then assign to the lvalue|
|>>>=|Right shift the lvalue (highest bit involved in shifting), then assign to the lvalue|
|&&=|Assign the rvalue to the lvalue when the lvalue is converted to a boolean value and is false|
|\|\|=|Assign the rvalue to the lvalue when the lvalue is converted to a boolean value and is true|
|~=|Match the lvalue with the rvalue, and assign the matched result to the lvalue|


Syntax Description:
```
self_operation_assignment_expression: assignment_left_value self_operation_assignment_operator expression

self_operation_assignment_operator: "+="
    | "-="
    | "*="
    | "/="
    | "%="
    | "**="
    | "|="
    | "&="
    | "^="
    | "<<="
    | ">>="
    | ">>>="
    | "&&="
    | "||="
    | "~="
```