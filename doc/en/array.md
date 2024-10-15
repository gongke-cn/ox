# Arrays
## Declaration
An array consists of a set of ordered elements. Each element can hold a value. Elements in an array can be referenced by a positive integer index value.

In OX, an array can be declared with the following statement:
```
[0,1,2,3]
```
The statement above declares a 4-element array, where the element value expressions are enclosed in "[" and "]", and each element value expression is separated by a ",".
If an element expression is empty, it indicates that the value of this element is null. For example:
```
[0,,1] //equivalent to [0,null,1]
```
If an array is declared across multiple lines with elements on different lines, the "," can be omitted. For example, the following statement is completely equivalent to the array declaration statement above:
```
[
    0
    1
    2
    3
]
```
## Element Expansion
We can add the contents of one array to another through element expansion. See the following code:
```
a = [1,2,3] //define variable a with the value array [1,2,3]
b = [0, ...a] //define variable b, add the element body of array a to b, the value of b is [0,1,2,3]
```
## Element Appending
If we have already defined an array variable, new elements can be added to the array through the element appending statement.
```
a = [] //declare variable a as a 0-element array
a.[0,1,2] //append elements to a, the value of a becomes [0,1,2]
b = [5,6]
a.[3,4,...b] //append elements to a, the value of a becomes [0,1,2,3,4,5,6]
```

Syntax Description:
```
array_literal: "[" array_items? "]"

array_items: array_items "," array_item
    | array_item

array_item:
    | expression
    | "..." expression

array_append_items: expression "." "[" array_items? "]"
```
## Array Length
We can get the number of elements in an array by accessing the "length" property of the array.
```
a = []
stdout.puts("{a.length}\n") //print 0
a.[1,2,3]
stdout.puts("{a.length}\n") //print 3
```
Setting the "length" property modifies the length of the array. If the new length is less than the original length, the array is truncated; if the new length is greater than the original length, the array is expanded, and the new elements are initialized to null.