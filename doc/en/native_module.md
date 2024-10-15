# Native Modules
In addition to writing scripts in the OX language, we can also compile C language code to generate binary libraries for invocation by the OX language.
Such modules in the form of binary libraries that can be called by the OX language are referred to as native modules.

A native module is essentially a dynamic link library with the suffix ".oxn". It contains the following two symbols:

* ox_load: Invoked when the module is loaded.
* ox_exec: The entry function of the module.

## ox_load
```
OX_Result ox_load (OX_Context *ctxt, OX_Value *s);
```
After loading the module, the OX interpreter calls this function. Typical operations in this function include adding references to other scripts and setting up the public symbol table of the current module.

## ox_exec
```
OX_Result ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv);
```
This function serves as the entry point for the module script. In this function, relevant classes, objects, and functions are generally created, and the public symbol table is initialized.

## Example
The following example implements a native module. The module references the "FILE" symbol from "std/io" and implements a function named "test".
```
#include <ox.h>

//Module reference array
static const OX_RefDesc
ref_tab[] = {
    //{module name, referenced symbol name, local symbol name}
    {"std/io", "FILE", "FILE"}, //Reference the symbol "FILE" from "std/io", local symbol name is "FILE"
    {NULL, NULL, NULL}
};

//Public symbol name array
static const char*
pub_tab[] = {
    "test", //Function "test"
    NULL
};

//Symbol index array.
//The front of the symbols should be public symbol indices, whose order must be consistent with the definitions in the public symbol name array pub_tab.
//Followed by referenced local symbol indices, whose order must be consistent with the definitions in the module reference array ref_tab.
//Finally, some internally defined local symbols can be added.
enum {
    ID_test, //Function "test"
    ID_FILE, //Symbol "FILE" referenced from "std/io"
    ID_MAX
};

//Script basic description
static const OX_ScriptDesc
script_desc = {
    ref_tab, //Module references
    pub_tab, //All public symbol names
    ID_MAX   //Total number of symbols
};

//Module loading
OX_Result
ox_load (OX_Context *ctxt, OX_Value *s)
{
    //Set the basic information of the module script and allocate the public symbol table
    ox_script_set_desc(ctxt, s, &script_desc);
    return OX_OK;
}

//test function implementation
static OX_Result
test_wrapper (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    //Get the symbol "FILE"
    OX_Value *file = ox_script_get_value(ctxt, f, ID_FILE);
    ...
    return OX_OK;
}

//Module execution entry function
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_value_stack_push(ctxt);

    //Create the function "test"
    ox_named_native_func_new_s(ctxt, v, test_wrapper, NULL, "test");
    //Bind the function "test" to the public symbol "test"
    ox_script_set_value(ctxt, s, ID_test, v);

    ox_value_stack_pop(ctxt, v);
    return OX_OK;
}
```
Run the following commands to compile and generate the local module "test.oxn":
```
gcc -o test.o -c -fPIC test.c
gcc -o test.oxn -shared test.o -lox -lm -lpthread -ldl -lffi
```
Write a script to invoke "test.oxn":
```
ref "./test"

test()
```