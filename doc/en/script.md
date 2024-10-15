# Script
## Running Scripts
We create a script file named "test.ox":
```
//test.ox

sum = 0

for i = 1; i < argv.length; i += 1 {
    sum += Number(argv[i]) //Convert parameters to numbers and accumulate
}

return sum
```
Execute this script with the following command:
```
ox --pr test.ox 1 2 3 4 5 6 7 8 9 10
```
The option "--pr" means printing the return value of the script execution. The printed result is:
```
55
```
When running the script, the OX interpreter parses and compiles "test.ox" and generates a script object. The script object contains an entry function whose code corresponds to the statements in the script.
Then the OX interpreter calls this entry function, and converts the command-line arguments "test.ox 1 2 3 4 5 6 7 8 9 10" into a string array ["test.ox", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"],
which is passed as a parameter to the entry function. After the entry function is executed, the return value is returned to the OX interpreter. Since the "--pr" option is used during runtime, the interpreter prints this result.

The process of running a script is as follows:

* Compile the script to generate a script object
* If the script object contains references to other scripts, execute the reference process for each reference.
* Run the entry function of the script

## References
Scripts can reference functions defined in other scripts through reference statements.

We create a file "test.ox" and define a function constant "test" in the file. Note that the "public" keyword is added before "test", indicating that this symbol can be referenced by other scripts.
```
//test.ox
ref "std/io"

public test: func() {
    stdout.puts("test!\n")
}

public test1: func() {
    stdout.puts("test1!\n")
}
```
We then create another file "run.ox" and add a reference statement in the file to reference the function "test" in "test.ox":
```
//run.ox
ref "./test" test as t //Reference "test" and save it as the local symbol "t"

t() //Call the "test" function
```
In the reference statement, "./test" is the name of the referenced script. Note that the script name should not have the ".ox" suffix, as the OX interpreter will automatically find and complete the file name. The script name can be in the following forms:

|Form|Example|Description|
|:-|:-|:-|
|Relative path|"./test" "../test"|Relative path name of the referenced script relative to the current script|
|Absolute path|"/home/ox/test" "c:/test"|Absolute path name of the referenced script|
|Library script in a software package|"std/io"|Reference a library script in a software package|
|Software package|"std"|Reference all library scripts in a software package|

In the reference statement, the reference symbol identifier before the keyword "as" represents the name of the public symbol to be imported from the referenced script.
The local symbol identifier after the keyword "as" represents the symbol name created in the local script. Note that this name should not conflict with the variable/constant names defined in the local script, otherwise an error will be reported.

If the referenced symbol name is the same as the local symbol name, we can omit the "as local symbol" part. The above code can be modified to:
```
//run.ox
ref "./test" test //Reference "test" and save it as the local symbol "test"

test() //Call the "test" function
```
If we want to import multiple symbols from a script, we can represent them in a pair of curly braces:
```
ref "./test" {test, test1 as t1}

test()
t1()
```
If we want to import all symbols from a script at the same time, we can write:
```
ref "./test" //Import all public symbols in "test.ox" and create internal symbols "test" and "test1"

test()
test1()
```
Alternatively, we can access the symbols in the script by accessing object properties:
```
ref "./test" * as t //Create an internal symbol "t", where t stores the script "test.ox", and "test" and "test1" are properties of the object t.

t.test()
t.test1()
```
The OX interpreter performs the following operations during the reference process:

* If the reference is an absolute or relative path name, it refers to a script file. Run this script file to obtain the object corresponding to the script.
* If the reference is a software package, it means referencing through the package manager
    * Load the software package information through the package manager, and throw an exception if the software package is not found
    * If the reference contains a script name, check whether the software package contains this library script
        - Throw a ReferenceError exception if the library does not exist
        - If the library script is found, run this script file to obtain the object corresponding to the script
    * If the reference does not contain a script name, it means referencing all libraries included in the software package
        - Create a script object
        - Run all library scripts in the software package in sequence, and add the public symbols in the library scripts to the newly created script object
* Import symbols in the script object
    * If the referenced symbol name is "*" and a local name n is specified, it means importing the script object.
        - Create a local symbol n and assign the script object to it
    * If the referenced symbol name is not "*", find whether the referenced symbol exists in the script object
        - Throw a ReferenceError exception if the referenced symbol is not found
        - If the referenced symbol is found, create a local symbol n and assign the value of the referenced symbol to it. n is the local symbol name specified in the reference; if no local symbol name is specified, n is the same as the referenced symbol
    * If there is no referenced symbol name, or the referenced symbol name is "*" and no local symbol is specified, it means importing all public symbols of the script
        - Traverse the public symbols in the script object. If the symbol is not defined locally, create a symbol with the same name locally and assign the corresponding value of the symbol in the script object to it

Note that when importing all symbols, only symbols that do not exist locally will be imported, and local variable values will not be overwritten. For example, the public symbol "test_value" is defined in "test.ox":
```
public test_value = 1978
```
Import all symbols from "test" in another script:
```
ref "./test"

test_value = 1
```
Since the variable "test_value" is already defined in the script, the symbol "test_value" in "test.ox" will not be imported.
To avoid conflicts between imported symbols and local variables, you can modify the local symbol name through the "as" keyword:
```
ref "./test" test_value as ext_test_value //ext_test_value = 1978

test_value = 1
```
Or you can specify to import the script object and access this symbol through the properties of the script object:
```
ref "./test" * as test //test.test_value = 1978

test_value = 1
```

## Built-in Script Methods
Scripts have the following built-in methods that can be used to traverse symbols in the script:

|Method|Description|
|:-|:-|
|keys()|Create an iterator to traverse all public symbol names in the script|
|values()|Create an iterator to traverse all public symbol values in the script|
|entries()|Create an iterator to traverse all public symbols in the script. The traversal return value is a 2-element array with values: [symbol name, symbol value]|
|$iter()|Has the same function as entries()|

The following code prints all public symbol names in the script "std/math":
```
ref "std/io"
ref "std/math" * as math

for math.keys() as key {
    stdout.puts("{key}\n")
}
```
## Syntax Description
```
script: script_items?

script_items: script_items script_item
    | script_item

script_item: statement
    | ref_statement
    | "public" expression

ref_statement: "ref" string_literal ref_declaration?

ref_declaration: "{" ref_items "}"
    | ref_item

ref_items: ref_items "," ref_item
    | ref_item

ref_item: identifier
    | identifier "as" identifier
    | "*"
    | "*" "as" identifier
```