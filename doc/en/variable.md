# Variables
## Variable Declaration
In the OX language, each variable is named with an identifier. The OX language references different variables through different identifiers.
An identifier used as a statement declares a variable with that identifier as its name. For example:
```
a //declare variable a
```
A newly declared variable has an initial value of null.

If you need to assign a value to a variable while declaring it, you can use an assignment statement for the identifier:
```
a = 1 //declare variable a and set its value to the number 1
```
After declaring a variable, you can reference it via its identifier or reassign it a new value:
```
a = 1 //declare variable a and set its value to the number 1

stdout.puts("value: {a}\n") //print the value of variable a, output "value: 1\n"

a = 2 //reassign variable a's value to 2

stdout.puts("value: {a}\n") //print the value of variable a again, output "value: 2\n"
```
Note that if a variable is referenced before its assignment statement, the variable's value is its initial value (null).
```
stdout.puts("value: {v}\n") //v is not assigned, its value is null, output "value: \n"
v = 1
stdout.puts("value: {v}\n") //output "value: 1\n"
```
Sometimes we want the value of a declared identifier to be unmodifiable; we can declare a constant using the following statement:
```
c: 3721 //declare constant c and set its value to the number 3721

c = 0 //Error: Constant c cannot be modified
```

Syntax Description:
```
variable_declaration: identifier ("=" expression)?

constant_declaration: identifier ":" expression
```
## Variable Scope
The scope of a variable is within the function it belongs to, see the following code:
```
my_func: func() {
    v = 1 //declare variable v, scope is within the my_func function
}

stdout.puts(v) //Error: Variable v is not in this scope
```
The variable v declared in the function my_func is only valid inside my_func and cannot be referenced outside the function.

In the OX language, functions can be nested, and inner functions can reference variables declared in outer functions. See the following code:
```
outter: func() {
    v = 1 //declare variable v

    inner: func() {
        stdout.puts(v) //inner function can access the variable declared by the outer function
    }

    inner() //call the inner function and print the value of variable v
}
```
The outer function declares the variable v, and the inner function can reference and access this variable.

Note that inner and outer functions can declare variables with the same name. See the following code:
```
outter: func() {
    v = 1 //declare variable v

    inner: func() {
        v = 2 //inner function also declares variable v

        stdout.puts("{v}\n") //print the value of v defined by the inner function: "2"
    }

    inner() //call the inner function
    stdout.puts("{v}\n") //print the value of v defined by the outer function: "1"
}
```
If a function uses an "identifier statement" or an "identifier assignment statement", it is considered that a new variable with this identifier is declared in the current function scope.
When an identifier is used to reference a variable in a program, the OX language first looks for the variable in the current function scope; if the corresponding variable is not found, it will look for variable definitions in outer functions in turn.

If an inner function wants to assign a value to a variable defined in an outer function (instead of declaring a new variable), you can add "@" before the variable identifier to indicate that it is an outer variable.
```
outter: func() {
    v = 1 //declare variable v

    inner: func() {
        @v = 2 //set the outer variable v
    }

    inner()
    stdout.puts("{v}\n") //v's value has been modified, print "2"
}
```
If an inner function needs to reference an outer variable multiple times, it only needs to add "@" once to mark it as an outer variable. For example:
```
outter: func() {
    v = 1 //declare variable v

    inner: func() {
        @v = 2 //set the outer variable v
        v = 3 //set the outer variable v
    }

    inner()
    stdout.puts("{v}\n") //v's value has been modified, print "3"
}
```