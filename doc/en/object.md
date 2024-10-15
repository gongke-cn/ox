# Objects
## Declaration
An object contains a set of properties. Each property consists of a string property name and a corresponding property value. See the following statement:
```
o = {a:1, b:2, c:3}
```
The properties of the object are defined between "{" and "}". Each property is separated by a ",". In a property definition, the part before ":" is the property name, and the part after ":" is the property value expression.
This statement declares an object and assigns it to the variable o. The object contains 3 properties: the property name "a" corresponds to the value 1, "b" corresponds to 2, and "c" corresponds to 3.
You can access the corresponding property value through the property name. If the object does not find a property with the corresponding name, it returns null:
```
stdout.puts("{o.a}\n") //prints 1
stdout.puts("{o.b}\n") //prints 2
stdout.puts("{o.c}\n") //prints 3
stdout.puts("{o.d}\n") //the "d" property is not defined, the result of "o.d" is null, printing an empty string
```
If declaring an object across multiple lines with properties on different lines, the "," can be omitted. For example, the following declaration:
```
{
    a:1
    b:2
    c:3
}
//equivalent to {a:1,b:2,c:3}
```
If the property name cannot be represented by an identifier, it can be directly represented as a string. For example:
```
id = 123

{
    "/": 0 //property name is "/"
    "?a": true //property name is "?a"
    "p{id}": 12 //property name is "p123"
}
```
In the program, you can also dynamically calculate the property name, for example:
```
pn = "prop"
o = {
    [pn+"1"]: 1 //equivalent to prop1: 1
    [pn+"2"]: 2 //equivalent to prop2: 2
    [pn+"3"]: 3 //equivalent to prop3: 3
}
```
The content between "[" and "]" is an expression, and the property name is the result of evaluating this expression.
## Property Spreading
We can add all properties of one object to another object through property spreading. See the following code:
```
o1 = {a:1, b:2}
o2 = {...o1, c:3} //o2 = {a:1, b:2, c:3}
```
## Property Appending
If a property is already defined in an object, assigning a value to this property can modify the property value. For example:
```
o = {a:1, b:2} //define an object and set properties "a" and "b"
o.b = o.a //the value of property "b" of o becomes 1
o.a = -1 //the value of property "a" of o becomes -1
```
If the assigned property is not defined in the object, the property will be automatically appended to the object and its value will be set. For example:
```
o = {} //no properties are defined in object o
o.a = 1 //add property "a" to object o with a value of 1
o.a = 2 //change the value of property "a" of object o to 2
```
If you want to append multiple properties, you can use the following method:
```
o = {a:1, b:2}
o.{
    c:3 //append property "c" with a value of 3
    d:4 //append property "d" with a value of 4
}
```

Syntax Description:
```
object_literal: "{" object_properties? "}"

object_properties: object_properties "," object_property
    | object_property

object_property: property_name ":" expression
    | "..." expression

property_name: identifier
    | string_literal
    | number_literal
    | "[" expression "]"

object_append_properties: expression "." "{" object_properties? "}"
```