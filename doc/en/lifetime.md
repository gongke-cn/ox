# Lifecycle
## Garbage Collector
OX manages memory through a garbage collector.

The garbage collector initiates the garbage collection process after the OX virtual machine has performed multiple memory allocations.
During the garbage collection process, the OX virtual machine scans and marks all memory that is still in use. Finally, it frees the memory that is not marked.
With the garbage collector, the OX virtual machine automatically detects and frees memory that is no longer in use, eliminating the need for users to manually release memory.
## $close
The timing of garbage collection initiation is controlled by the OX virtual machine itself, and users cannot manually control when the garbage collection process starts.
However, in actual development, there are times when certain resources need to be released immediately. Users can define resource release operations through the "$close" method.
For example, a file object automatically allocates and opens a file when created. When we finish file operations, we want to close the file immediately; otherwise, it may continue to occupy system file resources.
```
ref "std/io"

file = File("tmp.txt", "rb") //Open the file

do_file_operation(file) //File operation

file.$close() //Close the file
```
## Auto Close
We can declare variables as auto-close variables. In this way, when we reset the variable value or exit the function where the variable is located, the OX virtual machine will automatically call the variable's "$close" method to release the resources of the object stored in the variable.
```
ref "std/io"

#file = File("tmp.txt", "rb") //Adding "#" before the variable indicates it is an auto-close variable.

do_file_operation(file) //File operation

file = File("tmp2.txt", "rb") //Reset the variable file. Since file is an auto-close variable, "file.$close()" is called automatically first, then file is set to the new value

do_file_operation(file) //File operation

//Program ends, "file.$close()" is called automatically first
```