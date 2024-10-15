# Hello World
Let's start by writing your first OX script!  
Open your text editor and write the following code:
```ox
ref "std/io"

stdout.puts("hello, world!\n")
```
Save this script as "hello_world.ox". Then run the script with:  
```shell
ox hello_world.ox
```
You should see the terminal print:  
```shell
hello, world!
```
Let's break down what this script does. The first line:  
```ox
ref "std/io"
```
This is an OX language *reference statement*, indicating the script imports functionality from another script.  
`"std/io"` specifies the referenced script—`std` is the package name, `io` is the script filename within that package (separated by `/`).  
This line imports the input/output module (`io`) from OX's standard package (`std`).

The next line:  
```ox
stdout.puts("hello, world!\n")
```
`stdout` is the standard output object defined in `"std/io"`, and `puts` is its method for printing strings. The `"hello, world!\n"` inside the parentheses is the argument passed to `puts`.  
This line calls the `puts` method on `stdout`, printing `"hello, world!"` to the terminal.