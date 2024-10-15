# Exceptions
When a program runs into an error, the OX program usually throws an exception object through the "throw" statement.
The OX language internally defines the following exception object classes:

|Name|Description|
|:-|:-|
|Error|General error, parent class of other exception objects|
|NullError|Expected a valid value but got null instead|
|TypeError|Data type mismatch|
|RangeError|Out of valid range|
|SystemError|System error|
|ReferenceError|Invalid reference error|
|NoMemoryError|Insufficient memory space|
|TypeError|Data type mismatch|
|AccessError|No access permission|
|SyntaxError|Syntax error|

## Error
"Error" is the parent class of other errors. It defines the following methods:

* $init: Exception object instance initialization function, automatically called when creating an exception object instance.
    + Parameters:
        - message: Error message string. This string is stored in the "message" property of the exception instance.
* $to_str: Converts the exception object instance to a string. The string format is "Exception name: Error message".

## Custom Exceptions
Users can define their own exceptions in the program. For example:
```
//Custom exception，inherits "Error"
MyError: class Error {
}

try {
    ...
} catch e {
    if e instof MyError {
        // Catch the custom exception
    }
}
```
Users can also add their own defined properties to the exception object:
```
//Custom exception，inherits "Error"
MyError: class Error {
    //Add filename and line number to the error instance
    $init(msg, file, line) {
        //Initialize the base properties
        Error.$inf.$init(this, msg)
        //Store the filename and line number
        this.{
            file
            line
        }
    }

    //Convert to string
    $to_str() {
        s = Error.$inf.$to_str(this)

        //Add filename and line number to the string
        return "{file}: {line}: {s}"
    }
}
```