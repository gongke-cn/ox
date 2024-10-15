### Example
#### Terminal Printing
Print "hello, world!" to the terminal.
```
//Reference the standard input/output library
ref "std/io"

//Print to standard output
stdout.puts("hello, world!\n")
```
### Text Processing
Replace all "NAME" in a text file with "Zhang San".
```
//Reference the standard input/output library
ref "std/io"

//Open the file
#file = File("input.txt", "rb")

//Iterate through all lines in the file
for file.$iter(($.gets())) as line {
    //Replace "NAME" with "Zhang San"
    nline = line.replace(/\bNAME\b/, "Zhang San")
    //Output the replacement result
    stdout.puts(nline)
}
```
### Calling C Interface
Call the cURL library for HTTP communication.
```
//Reference the cURL module
ref "curl"
//Reference the standard input/output library
ref "std/io"

//cURL global initialization
curl_global_init(CURL_GLOBAL_DEFAULT)

//Create a cURL easy session.
curl = curl_easy_init()

//Set the URL
curl_easy_setopt(curl, CURLOPT_URL, "https://baidu.com")

//Write data callback function
write_fn: func(buf, size, num) {
    stdout.puts("recv: {buf.$to_str(0, num)}\n")
    return num
}.to_c(C.func_type(Size, UInt8.pointer, Size, Size))

//Set the write data callback function
curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_fn)

//Execute the HTTP session
curl_easy_perform(curl)

//Release the session
curl_easy_cleanup(curl)

//cURL global release
curl_global_cleanup()

```