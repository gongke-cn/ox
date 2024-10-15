# Functions
## Function Declaration
A function can be declared using the keyword "func". A function takes a set of parameters and returns a return value.
```
my_func: func (a, b, op) {
    if op {
        return a + b
    } else {
        return a - b
    }
}
```
The above program defines a function and assigns the function to the constant "my_func".
The function accepts 3 parameters: "a", "b", and "op".
In the function, addition or subtraction operations are performed on parameters "a" and "b" according to the value of the boolean parameter "op".
Finally, the result is returned through the "return" statement.

Next, we call this function through the constant "my_func".
```
stdout.puts("{my_func(1, 2, true)}\n") //prints 3
stdout.puts("{my_func(1, 2, false)}\n") //prints -1
```
## Parameters
Function parameters are a list of formal parameters enclosed in a pair of parentheses. If a function has no formal parameters, the parentheses can be omitted directly:
```
my_func: func {
    return "function without parameters"
}
```
A parameter can have a default value expression. When the parameter is not set or its value is null during function invocation, the default value is used as the parameter value.
```
my_func: func(a=100) {
    return -a
}

stdout.puts("{my_func()}\n") //prints -100
stdout.puts("{my_func(null)\n}") //prints -100
```
If a function accepts a variable number of parameters, the incoming parameter array can be accessed inside the function using the keyword "argv":
```
my_func: func() {
    for argv as arg {
        stdout.puts("{arg} ")
    }
    stdout.puts("\n")
}

my_func(1, 2, 3) //prints 1 2 3
my_func(1, 2, 3, 4, 5) //prints 1 2 3 4 5
```
If the first few parameters of a function are fixed, followed by a set of variable-length parameters, it can be defined in the following way:
```
//Parameters a and b are fixed parameters, and the subsequent variable parameters are stored in an array and assigned to the parameter args
my_func: func(a, b, ...args) {
    stdout.puts("a:{a} b:{b} args:")
    for args as arg {
        stdout.puts("{arg} ")
    }
    stdout.puts("\n")
}

my_func(1,2) //a=1, b=2, args=[]
my_func(1,2,3,4,5) //a=1, b=2, args=[3,4,5]
```
If you want to pass the elements of an array as variable parameters to a function, you can use "..." to expand the array elements into parameters:
```
a = [1,2,3]
my_func(...a) //equivalent to my_func(1,2,3)
```
Sometimes we need to pass an array as a parameter to a function:
```
my_func = func(arg) {
    return arg[0] + arg[1]
}

my_func([123, 321])
```
At this time, we can directly write the formal parameter in the form of an array, so that we can directly replace the access to array elements with the parameter variable name in the function body:
```
my_func = func([a, b]) {
    return a + b
}

my_func([123, 321])
```
Sometimes we need to pass an object as a parameter to a function:
```
my_func = func(arg) {
    return arg.p1 + arg.p2
}

my_func({p1:100, p2:200})
```
At this time, we can directly write the formal parameter in the form of an object, so that we can directly replace the access to object properties with the parameter variable name in the function body:
```
my_func = func({p1:a, p2:b}) {
    return a + b
}

my_func({p1:100, p2:200})
```
If the object's property names are the same as the parameter variable names, we can further simplify the above function to:
```
my_func = func({p1, p2}) {
    return p1 + p2
}

my_func({p1:100, p2:200})
```
## this Parameter
If a function is the value of an object's property, when the function is called by accessing the object's property value, the object can be accessed in the function through the this parameter.
```
o = {
    p: ""

    f: func() {
        stdout.puts("{this.p}\n")
    }
}

o.f() //o is passed to the function as the this parameter, and the function prints o.p
```
## Callback Functions
A function definition can be passed as a parameter to another function. This means using this function parameter as a callback function. For example:
```
test_cb: func(cb) {
    for i = 0; i < 100; i += 1 {
        cb(i) //call the callback function
    }
}

//call test_cb and print in the callback function
test_cb(func(i) {
    stdout.puts("{i}\n")
})
```
If the callback function performs a simple operation, an expression can be used to represent a callback function. For example:
```
//The function traverses array elements and modifies element content through a callback function
map: func(a, cb) {
    for i = 0; i < a.length; i += 1 {
        a[i] = cb(a[i])
    }
}

a = [0, 1, 2, 3, 4, 5, 6, 7]

//traverse the array and take the negative value of each element
map(a, (-$))
```
The callback function is encapsulated in a pair of parentheses, and the expression inside the parentheses represents the content executed by the callback function. The above call is equivalent to:
```
map(a, func(v) {
    return -v
})
```
In the definition of an expression callback function, "$" represents the first parameter of the function. To access other parameters, "$N" can be used, where "N" is the index number of the parameter. For example:
```
//Bubble sort
sort: func(a, cmp) {
    for i = 0; i < a.length; i += 1 {
        for j = i + 1; j < a.length; j += 1 {
            if cmp(a[i], a[j]) > 0 {
                t = a[i]
                a[i] = a[j]
                a[j] = t
            }
        }
    }
}

a = [9,1,7,6,3,5,0]

//Sort from largest to smallest
sort(a, ($1 - $0))
```
## Function Body
The function body is a set of internal statements enclosed in a pair of curly braces. If the operation to be performed by the function is simple and can be described by an expression, the function can be expressed in the following form:
```
my_func: func(a, b) => a + b
```
The expression to the right of "=>" corresponds to the function body, and the above definition is equivalent to:
```
my_func: func(a, b) {
    return a + b
}
```
## Internal Variables
Variables or constants declared inside a function have a scope limited to the inside of the function:
```
my_func: func() {
   v1 = 1 //declare internal variable v1 and assign it 1
   v2 //declare internal variable v2 with value null
   c: true //internal constant c with value true
}
```
When a function is defined inside another function, the inner function can access variables defined in the outer function:
```
outer: func() {
    outer_v = 1 //internal variable of outer function
    
    inner: func() {
        stdout.puts("{outer_v}\n") //access the variable outer_v defined in the outer function outer
    }

    inner()
}
```
If the inner function wants to modify the value of a variable defined in the outer function, note that an "@" should be added before the variable identifier. This lets the OX interpreter know that this is a variable defined in the outer function; otherwise, it will be considered a variable defined by the inner function itself:
```
outer: func() {
    outer_v = 1 //internal variable of outer function
    
    inner: func() {
        @outer_v = 2 //change the value of the variable defined in the outer function to 2
    }

    inner()

    inner2: func() {
        outer_v = 2 //inner2 defines its own internal variable outer_v, which will not affect the variable outer_v of the outer function
    }

    inner2()
}
```
If the inner function modifies the value of a variable defined in the outer function multiple times, it is sufficient to add "@" before the variable identifier in one place:
```
outer: func() {
    outer_v = 1 //internal variable of outer function
    
    inner: func() {
        @outer_v = 2 //change the value of the variable defined in the outer function to 2
        outer_v = 3 //change the value of the variable defined in the outer function to 3
    }

    inner()
}
```
## Function Invocation
When a function is invoked, the OX interpreter allocates a new data frame to store the function's parameters and internal variables.
```
generator: func(a) {
    v = a

    return func => v
}

f1 = generator(1) //return a function whose internal variable v is 1
f2 = generator(2) //return a function whose internal variable v is 2

stdout.puts("{f1()}\n") //prints 1
stdout.puts("{f2()}\n") //prints 2
```
## call Property
Each function contains a function property named "call", and the function can be invoked by calling this property.
The first parameter of "call" is passed to the function as the "this" parameter, and the subsequent parameters are passed to the function as regular parameters.
```
o: {
    test: func(a, b) {
        stdout.puts("{this.p} {a} {b}\n")
    }
}

o.test.call({p:1}, 2, 3) //this = {p:1}, argv = [2, 3] prints 1 2 3
```

## Syntax Description
```
function_declaration: "func" formal_parameters? function_body

formal_parameters: "(" formal_parameter_list? ")"

formal_parameter_list: formal_parameter_list "," formal_parameter
    | formal_parameter

formal_parameter: formal_parameter_pattern ("=" expression)?
    | "..." formal_parameter_pattern

formal_parameter_pattern: identifier
    | "[" array_parameter_pattern_items? "]"
    | "{" object_parameter_pattern_properties? "}"

array_parameter_pattern_items: array_parameter_pattern_items "," array_parameter_pattern_item
    | array_parameter_pattern_item

array_parameter_pattern_item:
    | formal_parameter_pattern ("=" expression)?

object_parameter_pattern_properties: object_parameter_pattern_properties "," object_parameter_pattern_property
    | object_parameter_pattern_property

object_parameter_pattern_property: property_name ":" formal_parameter_pattern ("=" expression)?
    | identifier

function_body: "=>" expression
    | "{" statements? "}"
```