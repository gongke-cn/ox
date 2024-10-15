# OX Language Environment Runtime Instructions
## Symbol Explanations
Note that some specific symbols are used in the following document, with the following definitions:

* %INSTALL_DIR%: The directory where the user installs the OX language environment.
* %TARGET%: The name of the target system architecture for OX environment installation. This name is in the format of "arch-vendor-os-abi". For example, "%TARGET%" for the x86 64-bit Linux platform is "x86_64-pc-linux-gnu". For the x86 32-bit Linux platform, "%TARGET%" is "i686-pc-linux-gnu". For the 64-bit Windows platform, "%TARGET%" is "x86_64-w64-windows-gnu".

## Directory Structure
The directory structure of the OX environment is similar to that of a Unix system. Under the OX language environment installation directory, there are the following subdirectories:

| Directory | Description |
|:-|:-|
| bin | Stores executable programs. On Windows systems, DLL libraries are also placed in this directory. |
| lib | Stores dynamic link libraries and static link libraries. |
| etc | Stores configuration files. |
| include | Stores header files. |
| share/ox/pkg | Stores installed OX software packages. Among them, the "all" directory stores software packages applicable to all architectures, and the "%TARGET%" directory stores software packages for specific system architectures. |
| share/ox/doc | Stores OX language-related documents. |

## Executing Programs
The OX language environment startup program is "%INSTALL_DIR%/bin/ox" on Linux systems and "%INSTALL_DIR%/bin/ox.exe" on Windows systems.
You can add the directory "%INSTALL_DIR%/bin" to the environment variable "PATH", so that you can directly enter "ox" to run the startup program.

Run the following command to view the version information of the OX language:
```
ox --version
```
## Running Scripts
Pass the path of the script file to be executed to the executable program to run the script. For example, to run the script program file "my_dir/my_program.ox", execute the following command:
```
ox my_dir/my_program.ox
```
If options need to be passed in when executing the script, they can be added after the script path. For example, the following command:
```
ox test.ox myarg1 myarg2 myarg3
```
The command will execute the script "test.ox", and the script will receive 4 string options: “test.ox“, ”myarg1", "myarg2", "myarg3”.

The OX executable program itself can also take options. Options before the script file path are options for the OX executable program, and options after the script file path are options for the script itself. As shown in the following command:
```
ox --log --log-file mylog.log test.ox myarg1 myarg2 myarg3
```
The command runs the script "test.ox". The part "ox --log --log-file mylog.log" are options for the OX executable program, and the part "test.ox myarg1 myarg2 myarg3" are options for the script "test.ox".

Sometimes users want to directly enter scripts on the command line, and can use the "-s" option to write scripts directly in the parameters. For example, execute the following command:
```
ox -s "ref \"std/io\"; stdout.puts(\"hello!\")"
```
The terminal displays:
```
hello
```
Sometimes users want to run a script and directly display the return value of the script, and can use the "--pr" option to print the return value of the script. For example, execute the following command:
```
ox --pr -s "1234+4321"
```
The terminal displays:
```
5555
```
Sometimes users want to execute a program in an installed software package instead of specifying the script file path. The "-r" option can be used, for example, execute the following command:
```
ox -r http/server
```
At this time, "http/server" represents "package name/script name" instead of the script file path. This command means running the script "server" in the installed software package "http".
Note that the script name should not have the ".ox" suffix. The executable program will automatically find the installed software packages and the scripts in the packages.

## Language Settings
OX supports multiple languages through gettext. Users can switch the output language of OX by setting the environment variable "LANG". On Linux, set the current language to Chinese:
```
export LANG=zh_CN.UTF-8
```
On Linux, set the current language to English:
```
export LANG=en_US.UTF-8
```
On Windows, set the current language to Chinese:
```
set LANG=zh_CN.UTF-8
```
On Windows, set the current language to English:
```
set LANG=en_US.UTF-8
```

## Logs
When an OX script is running, information can be printed through the log system to facilitate user debugging.

The log information printing level can be specified through the "--log" option.

| Parameter | Description |
|:-|:-|
| a | Output all log information |
| d | Output log information with level greater than or equal to debug level |
| i | Output log information with level greater than or equal to general information level |
| w | Output log information with level greater than or equal to warning level |
| e | Output log information with level greater than or equal to error level |
| f | Output log information with level greater than or equal to fatal error level |
| n | Turn off log printing |

Log information consists of multiple information fields separated by "|". For example, the following is the log information of "test.ox":
```
I|2025-11-24 15:59:48.843|1483079|test|"test.ox"|test_func|42|test start
E|2025-11-24 15:59:48.843|1483079|test|"test.ox"|test_func|56|test failed
```
If the user does not want to print information of certain fields, it can be specified through the parameters of the "--log-field" option. The parameter of "--log-field" is a string, where each character represents enabling an information field:

| Parameter | Description |
|:-|:-|
| l | Log level.<br>D: Debug information<br>I: General information<br>W: Warning information<br>E: Error information<br>F: Fatal error information<br> |
| d | Date, output format is yyyy-mm-dd |
| t | Time, output format is hh:mm:ss |
| m | Milliseconds |
| T | Log information tag |
| f | File name |
| F | Function name |
| L | Line number |
| i | Thread ID |

By default, log information is printed to the standard error output. If the user wants to print the log to a certain file, the "--log-file" option can be used:
```
ox --log a --log-file mylog.txt test.ox
```
The command runs the script "test.ox" and outputs the log to the file "mylog.txt".

## Usage and Options
To view the usage and options of the executable program, execute the following command:
```
ox --help
```
The common parameters of the executable program are as follows:

| Option | Parameter | Description |
|:-|:-|:-|
| --ast | | Output the abstract syntax tree of the script in JSON format. |
| --bc | | Output the bytecode instructions corresponding to the script. |
| -c | | Only compile the script, do not run the program. |
| -d | DIR | Add a software package search directory. If the "-d" option is not specified, OX searches the two directories "%INSTALL_DIR%/share/ox/all" and "%INSTALL_DIR%/share/ox/%TARGET%". If the "-d" option is specified, OX looks for software packages in the directory specified by the "-d" option. This parameter can be used multiple times. |
| --dump-throw | | Print stack information when an error is thrown. By default, OX only prints the stack information when the error occurs when the thrown error is not caught by the program. Specifying this option, OX prints the stack information immediately when the program throws an error, which can help developers quickly locate the location where the error occurs. |
| --enc | ENCODING | Set the character encoding of the input script file. By default, the default character encoding of OX is "UTF-8". |
| --help | | Display help information. |
| --log | a\|d\|i\|w\|e\|f\|n | Set the log output level. |
| --log-field | FIELD | Set the information fields for log output. |
| --log-file | FILE | Set the log output file name. |
| -p | | Only parse the file into an abstract syntax tree, do not compile. |
| --pr | | Print the program running result. |
| -r | | Load an executable program managed by the package manager. |
| -s | SOURCE | Run the script string SOURCE. |
| --version | | Display the OX version number. |