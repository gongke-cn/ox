# C Types
In the OX language, to facilitate mutual calls with native functions implemented in the C language, support for C types is provided.

C types include the following categories:

* Basic types: Including integer and floating-point data types
* Pointer types: Pointers and arrays
* Function types

## Basic Types
The OX language provides the following C basic data types.

| Object | Corresponding C Language Data Type | Description |
|:-|:-|:-|
|Int8|int8_t|8-bit signed integer|
|Int16|int16_t|16-bit signed integer|
|Int32|int32_t|32-bit signed integer|
|Int64|int64_t|64-bit signed integer|
|UInt8|uint8_t|8-bit unsigned integer|
|Uint16|uint16_t|16-bit unsigned integer|
|UInt32|uint32_t|32-bit unsigned integer|
|UInt64|uint64_t|64-bit unsigned integer|
|Float32|float|32-bit floating-point number|
|Float64|double|64-bit double-precision floating-point number|
|Size|size_t|Unsigned integer with a value length equal to the pointer length|
|SSize|ssize_t|Signed integer with a value length equal to the pointer length|
|Void|void||

By calling a C type object, you can create a corresponding C value object:
```
i8 = Int8() //Create an 8-bit signed integer with an initial value of 0
u32 = Uint32() //Create a 32-bit unsigned integer with an initial value of 0
f64 = Float() //Create a 64-bit double-precision floating-point number with an initial value of 0
```
To modify a C value, you need to assign a value to its "value" property:

```
i32 = Int32() //Create a 32-bit signed integer with an initial value of 0
i32.value = 1982 //Set the value of the 32-bit signed integer to 1982
```
Note that you should not directly assign a value to a variable that stores a C value. This operation will reassign the variable to an OX numeric value instead of a C value. For example:
```
v = Int32() //Create a 32-bit signed integer with an initial value of 0
stdout.puts("{typeof v}\n") //Print "Object:Int32"
v = 1982
stdout.puts("{typeof v}\n") //Print "Object:Number"
```
Basic types support the "$to_num()" method, which can be used to convert to an OX numeric value.
```
a = Int32()
a.value = 1978
n = a.$to_num() //Convert the unsigned integer a to an OX numeric value n

n = a + 1 //The addition operation implicitly calls a.$to_num(), and the result of n is the OX numeric value 1979
```
OX numeric objects include the following methods to convert numeric values to C basic types:

| Method | Returned C Type |
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
a = 1978.to_int32() //a is the 32-bit signed number 1978
b = 3.1415926.to_float32() //b is the 32-bit floating-point number 3.1415926
```
## Pointer Types
Through the "&" operator, we can get the pointer of a C type value. Through the "*" operator, we can get the value stored in a C pointer.
Syntax description:
```
get_pointer_expression: "&" expression

get_value_expression: "*" expression
```
For example:
```
i32 = Int32()
i32.value = 1978
ptr = &i8 //Get the pointer
stdout.puts(*ptr) //Print 1978
```
If a C type object is called with a numeric parameter, it means creating a C value array and returning a pointer to the array:
```
a = Int32(256) //Create a 32-bit signed number array with a length of 256
```
Elements of a C type array can be set and read:
```
a = Int32(256) //Create a 32-bit signed number array with a length of 256
for i = 0; i < 256; i += 1 {
    a[i] = i //Set array elements
}
for i = 0; i < 256; i += 1 {
    stdout.puts("{a[1]} ") //Read array elements
}

for a as i {
    stdout.puts("{i} ") //Traverse and print array elements
}
```
The length of a C value array can be obtained by calling the function "C.get_length":
```
a = Int32(256)
stdout.puts(C.get_length(a)) //Print 256
```
If the length of the array corresponding to the pointer is unknown, "C.get_length" returns -1.
If the parameter of "C.get_length" is not a pointer, the function throws a TypeError exception.

A pointer to an internal element of the array can be created by calling the function "C.get_ref". For example:
```
a = UInt8(256)

ref = C.get_ref(a, 200, 1) //ref is a pointer to a[200]
*ref = 0xff //Assign a value to a[200] through the ref pointer

ref = C.get_ref(a, 0, 2) //ref is an array pointer with the same address as a and a length of 2
ref[0] = 1 //Assign a value to a[0] through the ref pointer
ref[1] = 2 //Assign a value to a[1] through the ref pointer
```
If the element type of the pointer type is UInt8 or Int8, the "$to_str()" method can be called to convert the value to an OX string.
The "$to_str()" function takes two parameters:

* pos: The index of the first character of the string in the character array pointer. The default value is 0.
* len: The length of the string. If this parameter is not provided, the string starts from pos and ends at a position where NUL(0) is stored.

For example:
```
a = Int8(4)
a[0] = 'a'
a[1] = 'b'
a[2] = 'c'
a[3] = 0

s = a.$to_str(0, 1) //Create the string "a"
s = a.$to_str(0, 2) //Create the string "ab"
s = a.$to_str() //Create the string "abc"
s = a.$to_str(1) //Create the string "bc"
```
## Function Types
A function type internally includes a return value type and a set of parameter types.
A function type can be created by calling the function "C.func_type".
The first parameter of "C.func_type" represents the return value type of the function, and other parameters represent the parameter types of the function.
```
ftype = C.func_type(Void, Int32, Int32) //Create a function type void (*func) (uint32_t, uint32_t)
```
Functions written in OX can be converted to C functions by calling the "to_c()" method. This method can be used to write OX functions and use them as callback functions for C functions.
The parameter of the "to_c()" method is a function type object, indicating the type to convert to a C function.
For example:
```
//my_cb is a function of type int32_t (*cb) (int32_t, int32_t)
my_cb = func(a, b) {
    return a - b
}.to_c(C.func_type(Int32, Int32, Int32))

//Pass my_cb as a callback function to the C function c_func
c_func(100, my_cb)
```
## C Type Properties
All C types support the following properties:

| Name | Description |
|:-|:-|
|pointer|Get the pointer type corresponding to this type|
|size|Get the space occupied by the value of this type (in bytes). If the type size is unknown, return -1|
|length|If the type is a C array, return the number of array elements. If the number of array elements is unknown, return -1. If it is not an array, return 1.|

```
stdout.puts(Int8.size) //Print 1
stdout.puts(UInt64.size) //Print 8
stdout.puts(UInt64.pointer.size) //Print 8 on 64-bit systems and 4 on 32-bit systems
```