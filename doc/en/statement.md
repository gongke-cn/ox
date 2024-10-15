# Statements
## Syntax Description
```
statement: if_statement
    | case_statement
    | while_statement
    | do_while_statement
    | for_statement
    | for_as_statement
    | try_statement
    | break_statement
    | continue_statement
    | return_statement
    | expression
    | sched_statement

statements: statements statement
    | statement
```
## Statement Separators
In the OX language, two statements can be separated by a semicolon ";". For example:
```
a=1; b=2; c=3
```
## Line Breaks
If a line break is inserted between two statements, the statement separator ";" can be omitted. For example:
```
a=1;
b=2;
c=3
```
is equivalent to:
```
a=1
b=2
c=3
```
In addition, a long statement can also be split into multiple lines.
When the OX interpreter reads the last symbol at the end of a line, if this symbol cannot serve as a statement termination identifier according to the syntax, the OX interpreter continues to read symbols from the next line for parsing.
If the last symbol at the end of the line can be used as a statement termination identifier, the OX interpreter will automatically consider the current statement as ended.
For example:
```
a = 1 //"a=1" is a valid statement, and this line of statement ends here
a = 1 + //"+" cannot be the last symbol of a statement, continue reading the next line
  2     //The statement ends here, equivalent to a = 1 + 2
```
## Branch Statements
### if Statement
Converts the value of an expression to a boolean value, and executes the code block if the boolean value is true:
```
//if ...
if a == 1 {
    stdout.puts("ok\n")
}

//if ... elif ...
if a == 1 {
    stdout.puts("1\n")
} elif a == 2 {
    stdout.puts("2\n")
}

//if ... elif ... else ...
if a == 1 {
    stdout.puts("1\n")
} elif a == 2 {
    stdout.puts("2\n")
} elif a == 3 {
    stdout.puts("3\n")
} else {
    //Execute when none of the above conditions are met
    stdout.puts("other\n")
}
```
The if statement can also be used as an expression, where the value of the expression corresponds to the value of the last statement in the actually executed code block. For example:
```
v = if a == 1 {
    stdout.puts("a == 1\n")
    1000 //Set v=1000 when a==1
} else {
    stdout.puts("other\n")
    2000 //Set v=2000 when a!=1
}
```

Syntax Description:
```
if_statement: "if" expression "{" statements? "}" ("elif" expression "{" statements? "}")* ("else" "{" statements? "}")?
```
### case Statement
Evaluates the value of an expression, compares this value with branch conditions, and executes the corresponding code block when the comparison result is true. The branch condition "*" indicates that this code block is executed when none of the other conditions are met:
```
case a {
1 {
    //a == 1
    stdout.puts("1\n")
}
2
3 {
    //a == 2 or a == 3
    stdout.puts("2 or 3\n")
}
* {
    //Execute when none of the above conditions are met
    stdout.puts("other\n")
}
}
```
In general, the case statement compares whether the value of the expression is equal to the value of the branch condition, and executes the corresponding code block if they are equal.
The case statement can also use a complex expression as a branch condition, and executes the corresponding code block when the return value of the expression is converted to a boolean value of true.
```
case a {
($ > 3)
($ < 5) {
    //a > 3 && a < 5
    stdout.puts("a > 3 && a < 5\n")
}
($ > 2) {
    //a > 2
    stdout.puts("a > 2\n")
}
($ > 1) {
    stdout.puts("a > 1\n")
}
* {
    stdout.puts("other\n")
}
}
```
A branch condition enclosed in a pair of parentheses represents using the expression evaluation method for branch comparison, where "$" in the expression represents the value of the expression "a" used by the case for comparison.

The case statement can also be used as an expression, where the value of the expression corresponds to the value of the last statement in the actually executed code block. For example:
```
//When a is greater than 0, v is 1; when a is less than 0, v is -1; when a is equal to 0, v is 0
v = case a {
($ > 0) {
    1
}
($ < 0) {
    -1
}
0 {
    0
}
}
```

Syntax Description:
```
case_statement: "case" expression "{" case_items? "}"

case_items: case_conditions "{" statements? "}"

case_conditions: case_conditions case_condition
    | case_condition

case_condition: "*"
    | expression
    | "(" expression ")"
```
## Loop Statements
Loop statements include "while" statements, "do while" statements, "for" statements, and "for as" statements.
### while Statement
Executes the code block when the loop condition is met.
```
//Print 0 ~ 99
a = 0
while a < 100 {
    stdout.puts("{a}\n")
    a += 1
}
```

Syntax Description:
```
while_statement: "while" expression "{" statements? "}"
```
### do while Statement
The do while loop executes the code block first, then checks the loop condition. For example:
```
//Execute first, then check the condition, resulting in printing "0"
a = 0
do {
    stdout.puts("{a}\n")
} while a > 1
```

Syntax Description:
```
do_while_statement: "do" "{" statements? "}" "while" expression
```
### for Statement
```
for i = 0; i < 100; i += 1 {
    stdout.puts("{i}\n")
}
```
Looking at the code above, the loop condition of for is divided into three parts, separated by ";".
The first part is the initialization part, which is executed once before entering the loop.
The second part is the loop check condition, which is executed each time before running the code inside the loop; if the check condition is false, the loop exits immediately.
The third part is the data update part, which runs after the code inside the loop is executed each time.
All three parts are optional. If the second loop check condition is empty, it means the check condition is always true.

```
//Infinite loop
for ;; {
    stdout.puts("loop\n")
}
```

Syntax Description:
```
for_statement: "for" expression? ";" expression? ";" expression? "{" statements? "}"
```
### for as Statement
The for...as statement creates an iterator and loops through all values in the iterator.
```
a = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]

//Create an iterator to traverse all elements in the array.
for a as i {
    stdout.puts("{i}\n")
}
```
The execution process is as follows:

* The expression on the left side of "as" calls the "$iter()" attribute of the expression to create a new iterator.
* Loop
    1. Check the "end" attribute of the iterator; if end is true, exit the loop immediately
    2. Access the "value" attribute of the iterator to get the current value of the iterator
    3. Copy the current value of the iterator to the expression on the right side of "as"
    4. Execute the code block
    5. Jump to step 1
* Call the "$close()" attribute of the iterator to close the iterator

Syntax Description:
```
for_as_statement: "for" expression "as" reverse_assginment_left_value "{" statements? "}"
```
### break Statement
The break statement can appear in loop statements; when executed, it immediately jumps out of the outer loop. For example:
```
i = 0
while true {
    if i > 100 {
        //Jump out of the loop after printing 100 times
        break
    }
    stdout.puts("{i}\n")
    i += 1
}
```
### continue Statement
The continue statement can appear in loop statements; when executed, it immediately ends the execution of the code in the loop body and enters the next loop. For example:
```
//Print 0, 2, 4, 6, ... 98
for i = 0; i < 100; i += 1 {
    if i & 1 {
        //If i is an odd number, enter the next loop
        continue
    }
    stdout.puts("{i}\n")
}
```
Syntax Description:
```
continue_statement: "continue"
```
## throw Statement
Throws an exception; the OX interpreter immediately ends the current execution flow and jumps directly to the "catch" block in the outer "try" statement. At this point, the exception is caught, and the corresponding code in the "catch" block is executed.
If the thrown exception is not caught, the OX interpreter exits directly and prints the error and stack information.
```
//Throw a type error
throw TypeError("type error!")
```

Syntax Description:
```
throw_statement: "throw" expression
```
## try Statement
```
try {
    do_something()
} catch e {
    stdout.puts("error!\n")
}
```
When the try code block is executed, if an exception is thrown, it immediately enters the catch block. The thrown exception is assigned to the variable "e", and then the code in the catch block is executed to print the error.
If the try code block executes normally without generating an exception, the catch block part is ignored.

If the try statement needs to perform some identical operations in both abnormal and normal cases, a finally block can be added at the end of the try statement.
```
a = File()

try {
    do_something()
} finally {
    a.$close()
}
```
The code above adds a closing operation for file a in the finally block. Then, whether do_something() executes with an exception or not, file a will be closed.
If an exception is thrown in do_something(), OX first jumps to finally to execute the closing operation of file a, and then continues to throw the exception to the upper layer.

Syntax Description:
```
try_statement: "try" "{" statements? "}" ("catch" reverse_assginment_left_value "{" statements "}")? ("finally" "{" statements "}")?
```
## return Statement
Jumps out of the current function directly and gives the function return value. If no expression is added after the "return" statement, it means the return value is null.
```
func f1(a,b) {
    //Return the value of parameter a + parameter b
    return a + b
}

stdout.puts("{f1(1, 2)}\n") //Print 3

func f2 {
    //Return null
    return
}

stdout.puts("{f2()}\n") //Print empty string
```

Syntax Description:
```
return_statement: "return" expression?
```