# Software Packages
OX language manages libraries and applications developed with OX through software packages. Each package contains multiple script files.

Packages are placed in the "share/ox/pkg" subdirectory under the OX installation directory. When we open this directory, we can see the following subdirectories:

* all: Stores packages that can run on all platforms.
* x86_64-pc-linux-gnu: Stores packages that can run on 64-bit x86 platform Linux systems (x86_64 Linux environment).
* x86_64-w64-windows-gnu: Stores packages that can run on 64-bit x86 platform Windows systems (x86_64 Windows environment).

If we enter the subdirectory "all", we can see multiple subdirectories:
```
doc/    oxngen/ oxp/    pb/     pm/
```
Each subdirectory contains a software package. Here we can see that the packages "doc", "oxngen", "oxp", "pb", "pm" are currently installed.
## package.ox
Each software package contains a "package.ox" file, which describes the basic information of the package.

Let's look at the content of the "pkg/all/oxp/package.ox" file:
```
{
  "name": "oxp",
  "description": {
    "en": "OX package operation library",
    "zh": "OX脚本语言软件包操作库"
  },
  "version": "0.0.1",
  "architecture": "all",
  "homepage": "https://gitee.com/gongke1978/ox",
  "maintainer": "OX maintainer",
  "dependencies": {
    "ox": "0.0.1",
    "std": "0.0.1",
    "archive": "0.0.1"
  },
  "libraries": [
    "oxp",
    "package_schema",
    "package_list_schema"
  ],
  "files": [
    "%pkg%/package.ox",
    "%pkg%/oxp.ox",
    "%pkg%/package_schema.ox",
    "%pkg%/package_list_schema.ox",
    "%pkg%/log.ox",
    "%doc%/md/oxp/oxp.md",
    "%doc%/md/oxp/oxp_oxp.md"
  ]
}
```
This OX file is compatible with the JSON data format and contains the following descriptions:

|Name|Format|Description|
|:-|:-|:-|
|name|String|Name of the software package|
|description|Multilingual object|Basic description of the package, where the object property names represent languages and the property values are descriptions in the corresponding languages|
|version|String|Version of the software package|
|architecture|String|Running platform of the package, "all" means the package can run on all platforms|
|homepage|String|Homepage of the software|
|maintainer|String|Maintainer of the software|
|dependencies|Dependency object|Describes the package's dependencies on other packages. The object property names represent the dependent package names, and the property values represent the minimum version of the dependent packages|
|libraries|String array|Libraries in the package that can be referenced by external packages|
|executables|String array|Executable programs in the package|
|files|String array|List of all files in the package|

### Libraries
When writing an OX script, we can reference scripts from installed packages:
```
//The script references the "oxp" script in the "oxp" package
ref "oxp/oxp"
```
When the OX interpreter finds that the script references the package "oxp", it will look for the "oxp" subdirectory in the package installation directory and parse the "package.ox" file in it.
The "libraries" attribute in "package.ox" contains the "oxp" script, indicating that "oxp" includes this library script.
The OX interpreter then loads the "oxp/oxp.ox" script file and imports the symbols in it into the current script.

If the "oxp" subdirectory is not found, or the "package.ox" does not describe the library "oxp", the OX interpreter will throw a ReferenceError exception.

If a script wants to import all definitions from all libraries in a package, use the following method:
```
ref "oxp"
```
The above statement will import all symbols from the three scripts "oxp", "package_schema", and "package_list_schema" in the "oxp" package into the current script.
### Executable Programs
The "executables" attribute of "package.ox" defines the executable programs included in the package. For example, the "package.ox" in the "http" package is defined as follows:
```
{
  "name": "http",
  "description": {
    "en": "HTTP protocol",
    "zh": "HTTP超文本传输协议"
  },
  "version": "0.0.1",
  
  ...

  "executables": [
    "server"
  ],

  ...

}
```
It means the "http" package contains the executable script "server". We can execute this script with the following command:
```
ox -r http/server --help
```
The "-r" option of the ox program indicates that "http/server" is not a script file name, but a "package name/executable script name".
The ox program will find the package "http", load its "package.ox", and load the executable script according to the "executables" description in it.
The option "--help" followed in the command line is the parameter passed to the executable script "server.ox".

If the name of the executable script is the same as the package name, the "package name" can be used directly in the ox program options instead of "package name/executable script name".
For example, if we have a package "test" with the following definition in its "package.ox":
```
{
  "name": "test",
  "description": {
    "en": "test"
  },
  "version": "0.0.1",
  
  ...

  "executables": [
    "test"
  ],

  ...

}
```
We can directly run the following command to execute the script "test.ox":
```
ox -r test
```
## Package Management
OX provides a package manager "pm" to manage software packages.
Run the following command to view the help information of "pm":
```
ox -r pm --help
```
### List Installed Packages
The "-l" option of "pm" is used to list installed packages:
```
ox -r pm -l
```
Print installed software information:
```
archive              (0.0.1)         Library supporting data packaging and compression in multiple formats
curl                 (0.0.1)         curl - Network transfer library
doc                  (0.0.1)         Document generator
gettext              (0.0.1)         Message translation tool
http                 (0.0.1)         HTTP Hypertext Transfer Protocol
json                 (0.0.1)         JSON library
ox                   (0.0.1)         OX scripting language
ox_devel             (0.0.1)         OX scripting language development libraries and header files
oxngen               (0.0.1)         OX scripting language native module generator
oxp                  (0.0.1)         OX scripting language package operation library
pb                   (0.0.1)         OX scripting language package building tool
pm                   (0.0.1)         OX scripting language package manager
ssl                  (0.0.1)         OpenSSL - Secure Sockets and encryption library
std                  (0.0.1)         OX scripting language standard library
xml                  (0.0.1)         XML parsing library
```
The information includes three columns: the first column is the package name, the second column is the package version, and the third column is the package description.
### View Installed Package Information
The "-q" option of "pm" is used to query package information. Run the following command to query the information of the package "oxp":
```
ox -r pm -q oxp
```
The information is printed as follows:
```
name: oxp
architecture: all
version: 0.0.1
description:
  OX scripting language package operation library
dependencies:
  ox (0.0.1)
  std (0.0.1)
  archive (0.0.1)
libraries:
  oxp
  package_schema
  package_list_schema
homepage: https://gitee.com/gongke1978/ox
maintainer: OX maintainer
```
### Install Packages from Server
The "-s" option of "pm" is used to synchronize and install packages from the server.
Run the following command to install or update the package "sdl" from the server:
```
ox -r pm -s sdl
```
The package manager will automatically check the package's dependencies and install or synchronize all required packages:
```
The following packages will be synchronized:
  "sdl" (0.0.1) size: 39849
Downloading "sdl-x86_64-pc-linux-gnu-0.0.1.oxp"
Package "sdl" downloaded to "/usr/share/ox/oxp/sdl-x86_64-pc-linux-gnu-0.0.1.oxp"
Package "sdl" (0.0.1) installed successfully
The installed package depends on the following system packages:
  libSDL-1.2_0
```
The server configuration files are located in the "share/ox/server/" subdirectory under the OX installation directory.
The file "main.ox" is the definition of the OX main package server.
Users can add their own server configurations in this directory.

If the user has already installed the package "sdl" and wants to update "sdl" to the latest version on the server, run the command:
```
ox -r pm -L -s sdl
```
The "-L" option means updating the package list from the server, and the "-s" option means synchronizing the package to the latest version.
Note that the package manager caches the server list locally, and adding the "-L" option will update the latest package list from the server.

Use the "--list-server" option to list all software lists on the server side:
```
ox -r pm --list-server
```

Use the "--query-server" option to view package information on the server side:
```
ox -r pm --query-server sdl
```
### Install Package Files
Sometimes we have obtained a package archive file with the ".oxp" suffix and want to install the package directly from the local file.
In this case, we can use the "--install" option to install the package directly:
```
ox -r pm --install my_package.oxp
```
### Remove Packages
Use the "-r" option to remove a package:
```
ox -r pm -r sdl
```
### Clean Packages
Use the "--clean" option to detect locally removable packages:
```
ox -r pm --clean
```
Removable packages refer to those packages that do not contain executable programs and are not dependent on by other packages.
### Repair
If an exception occurs during the previous package installation or removal process, it may cause some required packages to be missing locally.
Use the "--repair" option to detect local information, find missing packages, and reinstall them:
```
ox -r pm --repair
```
### System Packages
The operation of OX software not only depends on packages written in the OX language but also sometimes depends on some system libraries and system programs.
For example, the package "pb" depends on system packages such as "gcc", "gnu-gettext", and "pkgconf".
Users can install these system packages by themselves or through the system's built-in package management program, or use "pm" to install system software packaged by OX.
Note that if "pm" is used to install system software, there may be conflicts with the system's built-in package management program.
Generally, on Linux systems, it is recommended to use the system's built-in package manager to manage system software, such as apt on Ubuntu, dnf on Fedora, etc.
On Windows, if the user uses msys2 to install and manage OX, it is recommended to use msys2's pacman to manage system software.
If the user installs the OX environment separately on Windows, it is recommended to use "pm" to install and manage system packages.

If the user wants to automatically install the dependent system packages when installing a package, add the "--sys" option to the synchronization command:
```
ox -r pm -s --sys pb
```
With the above command, "pm" will automatically install system packages such as "gcc", "gnu-gettext", and "pkgconf" that "pb" depends on.
## Package Building
OX provides a package building tool "pb" to automatically build software packages.

### build.ox
To create a custom package, users need to write a "build.ox" file to describe the basic building methods of the package.
The "pb" program reads "build.ox" and calls tools such as "gettext", "doc", and "gcc" according to the described methods to automatically build the package.

Let's try to create a package "test" with "pb".

First, we create a "build.ox" file with the following content:
```
{
    name: "test" // Package name
    homepage: "HOMEPAGE" //Add the package's homepage here
    maintainer: "MAINTAINER" //This is the name of the package maintainer
    //This is the package description
    description: {
        //English description
        "en": ""
        //Chinese description
        "zh": ""
    }
    version: "0.0.1" //Version information
    //Package dependencies
    dependencies: {
        "std": "0.0.1" //Standard library package, minimum version is 0.0.1
    }
    //Packages that pb depends on during the build process
    development_dependencies: {
    }
    //Executable programs contained in the package
    executables: [
    ]
    //Library scripts provided by the package that can be referenced externally
    libraries: [
    ]
}
```
Then we create an executable script "test.ox" in the same directory with the following content:
```
ref "std/io"

stdout.puts(L"test!\n")
```
We add the script name to "build.ox":
```
{
    ...
    //Executable programs contained in the package
    executables: [
        "test"
    ]
    ...
}
```
### Build the Package
In the same directory, run the following command to build the package "test":
```
ox -r pb
```
We can see that the file "package.ox" is generated in the current directory, indicating that the build is completed.

We can also run the following command to build and package the package "test":
```
ox -r pb -p
```
We can see that the package "test-all-0.0.1.oxp" is generated in the current directory.
In this way, we can install this package through the "pm" program:
```
ox -r pm --install test-all-0.0.1.oxp
```
### Multilingual
Run the following command to update multilingual information:
```
ox -r pb --update-text
```
We find that "pb" has created an information template file "locale/test.pot".

If we want to support Chinese, run the following command:
```
ox -r pb --gen-po zh_CN
```
"pb" calls the "gettext" tool to create the Chinese translation information file "locale/zh_CN.po".

We can edit "locale/zh_CN.po" to translate the text information in it into Chinese:
```
#: test.ox:3
msgid "test!\n"
msgstr "测试!\n"
```
Then we rebuild the package:
```
ox -r pb -p
```
We find that the package automatically contains the Chinese information translation file "locale/zh_CN/LC_MESSAGES/test.mo".

We reinstall the package, and by setting the environment variable to "LANG=zh_CN.UTF-8" or "LANG=en_US.UTF-8", we can switch between Chinese and English display when running the program.
### Documentation
We can add documentation descriptions to the script:
```
ref "std/io"

/*?
 *? @package test Test.
 *? @exe Print test message
 */
stdout.puts(L"test!\n")
```
Rebuild the package:
```
ox -r pb -p
```
We find that the package contains the documentation markdown files "doc/md/test/test.md" and "doc/md/test/test_test.md".
After installation, we can view these documents in the "share/ox/doc/md/test" subdirectory under the OX installation directory.
### Native Modules
We can add build rules for native modules in the package.
Add the "oxn_modules" attribute to "build.ox" and include native build rules in it:
```
{
    ...
    "oxn_modules": {
        //Native module "test_oxn.oxn"
        "test_oxn": {
            cflags: "" //GCC compiler options
            libs: "" //Link options
            //C source files
            sources: [
                "test.oxn.c"
            ]

        }
    }
    ...
}
```
Rebuild the package:
```
ox -r pb -p
```
We find that "pb" calls gcc to compile and generate the native module "test_oxn.oxn".
### oxngen
"pb" can also call "oxngen" to scan header files and automatically generate native modules.
Add the "oxngen_targets" attribute to "build.ox" and include "oxngen"-related rules in it:
```
{
    ...
    oxngen_targets: {
        //Native module "test_oxngen.oxn"
        test_oxngen: {
            cflags: "" //GCC compiler options
            libs: "" //Link options
            input_files: [
                "test.h" //Scan "test.h"
            ]
        }
    }
    ...
}
```
Rebuild the package:
```
ox -r pb -p
```
We find that "pb" calls "oxngen" and gcc to generate the native module "test_oxngen.oxn".
### Cleanup
During the build process, "pb" generates some intermediate files. When building again, if "pb" finds that the intermediate files already exist and do not need to be updated, it will use these files directly.

If the user wants to clean up these intermediate files, they can run the following command to delete them:
```
ox -r pb --clean
```
### Build Parameters
Sometimes we need to add some options during the build process to adjust the build method through these options. We can achieve this through build parameters.
When calling "pb", we can add a build parameter by adding the "-s" option.
The parameter is in the form of "parameter name=parameter value". If the "=parameter value" part is not included, the parameter value is set to the boolean value true; if the "=parameter value" part is included, the parameter value is the string after "=".
For example:
```
ox -r pb -p -s DEBUG -s TAG=test
```
The above command defines two parameters "DEBUG" and "TAG". Among them, the value of DEBUG is true, and the value of TAG is "test".

We can access these parameters in build.ox in the following way:
```
config = argv[0] //The configuration is passed as a parameter to "build.ox", where config.p is the parameter object, and its properties correspond to parameter names, and property values are parameter values.

if config.p.DEBUG {
    cflags = "-g"
} else {
    cflags = "-O2"
}

if config.p.TAG {
    cflags += " -DTAG={config.p.TAG}"
}

{
    name: "test"
    description: {
        "en": "test"
    }
    version: "0.0.1"
    libraries: [
        "test"
    ]
    "oxn_modules": {
        "test": {
            cflags: cflags
            sources: [
                "test.oxn.c"
            ]

        }
    }
}
```
In this way, we can control whether to enable debugging information during compilation through the DEBUG parameter, and modify the value of the macro definition "TAG" through the "TAG" parameter.