# 软件包
OX语言通过软件包管理OX开发的库和应用程序。每个软件包中包含了多个脚本文件。

软件包放置在OX安装目录下的"share/ox/pkg"子目录中。我们打开这个目录，可以看到下面几个子目录：

* all: 存放可以在所有平台下运行的软件包。
* x86_64-pc-linux-gnu: 存放可以在64位x86平台Linux系统上运行的软件包(x86_64 linux 环境)。
* x86_64-w64-windows-gnu: 存放可以在64位x86平台Windows系统上运行的软件包(x86_64 Windows 环境)。

我们再进入子目录“all”，可以看到多个子目录：
```
doc/    oxngen/ oxp/    pb/     pm/
```
每个子目录中都存放了一个软件包。这里我们看到当前已经安装了"doc", "oxngen", "oxp", "pb", "pm"这几个软件包。
## package.ox
每个软件包中，都包含一个"package.ox"文件，这个文件描述了软件包的基本信息。

我们看一下"pkg/all/oxp/package.ox"文件的内容：
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
这个OX文件兼容JSON的数据格式，其中包含以下描述：

|名称|格式|描述|
|:-|:-|:-|
|name|字符串|软件包的名称|
|description|多语言对象|软件包基本描述，对象属性名代表语言，属性值为对应语言的描述|
|version|字符串|软件包版本|
|architecture|字符串|软件包的运行平台，"all"表示软件包可在所有平台下运行|
|homepage|字符串|软件主页|
|maintainer|字符串|软件维护者|
|dependencies|依赖对象|描述软件包对其他软件包的依赖。对象属性名代表依赖的软件包名，属性值代表依赖软件包的最低版本|
|libraries|字符串数组|软件包中可以被外部软件包引用的库|
|executables|字符串数组|软件包中的可执行程序|
|files|字符串数组|软件包中全部文件列表|

### 库
当我们编写一个OX脚本时，我们可以引用已安装软件包中的脚本：
```
//脚本引用"oxp"软件包中的"oxp"脚本
ref "oxp/oxp"
```
当OX解释器发现脚本引用了软件包"oxp"时，会在软件包安装目录中查找"oxp"子目录，并解析其中的"package.ox"。
"package.ox"中"libraries"属性中包含了"oxp"脚本，表示"oxp"中包含这个库脚本。
然后OX解释器加载"oxp/oxp.ox"脚本文件，再将其中的符号引入当前脚本中。

如果没有找到"oxp"子目录，或者"package.ox"没有描述库"oxp"，OX解释器会抛出ReferenceError异常。

如果脚本要引用一个软件包中所有库中的定义，使用以下方法：
```
ref "oxp"
```
上面的语句会将"oxp"软件包中的"oxp", "package_schema", "package_list_schema"三个脚本中的全部符号引入当前脚本中。
### 可执行程序
"package.ox"的"executables"属性定义了软件包中包含的可执行程序。比如"http"软件包中"package.ox"定义如下：
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
表示"http"软件包中包含了可执行脚本"server"。我们可以通过下面的命令执行这个脚本：
```
ox -r http/server --help
```
ox程序的"-r"选项表示"http/server"不是一个脚本文件名，而是"软件包名/可执行脚本名"。
ox程序会查找软件包"http"并加载其"package.ox"，根据其中的"executables"描述加载可执行脚本。
命令行中后跟的选项"--help"是传递给可执行脚本"server.ox"的参数。

如果可执行脚本的名称与软件包的名称相同，则ox程序选项中可以直接用"软件包名"替代"软件包名/可执行脚本名"。
比如我们有一个软件包"test",其""package.ox"中定义如下：
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
我们可以直接运行下面的命令运行脚本"test.ox":
```
ox -r test
```
## 软件包管理
OX提供软件包管理器"pm"对软件包进行管理。
运行以下命令查看"pm"的帮助信息：
```
ox -r pm --help
```
### 列出已安装软件包
"pm"的选项"-l"用于列出已安装软件包：
```
ox -r pm -l
```
打印已安装软件信息：
```
archive              (0.0.1)         支持多种格式的数据打包和压缩库
curl                 (0.0.1)         curl - 网络传输库
doc                  (0.0.1)         文档生成器
gettext              (0.0.1)         信息翻译工具
http                 (0.0.1)         HTTP超文本传输协议
json                 (0.0.1)         JSON库
ox                   (0.0.1)         OX脚本语言
ox_devel             (0.0.1)         OX脚本语言开发库和头文件
oxngen               (0.0.1)         OX脚本语言原生模块生成器
oxp                  (0.0.1)         OX脚本语言软件包操作库
pb                   (0.0.1)         OX脚本语言软件包构建工具
pm                   (0.0.1)         OX脚本语言包管理器
ssl                  (0.0.1)         OpenSSL - 安全套接字及加密库
std                  (0.0.1)         OX脚本语言标准库
xml                  (0.0.1)         XML解析库
```
信息包括3列，第一列为软件包名称，第二列为软件包版本，第三列为软件包描述。
### 查看已安装软件包信息
"pm"的选项"-q"用于查询软件包信息。运行以下命令查询软件包"oxp"的信息：
```
ox -r pm -q oxp
```
信息打印如下:
```
name: oxp
architecture: all
version: 0.0.1
description:
  OX脚本语言软件包操作库
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
### 从服务器安装软件包
"pm"的选项"-s"用于从服务器同步安装软件包。
运行以下命令从服务器安装或更新软件包"sdl":
```
ox -r pm -s sdl
```
包管理器会自动检查软件包的依赖关系，安装或同步所需的全部软件包：
```
以下软件包将被同步:
  "sdl" (0.0.1) size: 39849
下载"sdl-x86_64-pc-linux-gnu-0.0.1.oxp"
软件包"sdl"下载为"/usr/share/ox/oxp/sdl-x86_64-pc-linux-gnu-0.0.1.oxp"
软件包"sdl" (0.0.1)安装完毕
已安装包依赖以下系统包:
  libSDL-1.2_0
```
服务器配置文件在OX安装目录的"share/ox/server/"子目录下。
其中文件"main.ox"是OX主软件包服务器定义。
用户可以在此目录下添加自己的服务器配置。

如果用户已经安装软件包"sdl",希望将"sdl"更新为服务器上的最新版本，运行命令：
```
ox -r pm -L -s sdl
```
选项"-L"表示从服务器更新软件包列表，选项"-s"表示将软件包同步到最新。
注意包管理器会将服务器列表缓存到本地，增加"-L"选项会从服务器更新最新的软件包列表。

使用"--list-server"选项可以列出服务器端的所有软件列表：
```
ox -r pm --list-server
```

使用"--query-server"选项可以查看服务器端软件包信息：
```
ox -r pm --query-server sdl
```
### 安装软件包文件
有时我们已经获取了".oxp"后缀的软件包打包文件，想直接从本地文件安装软件包。
此时我们可以使用选项"--install"直接安装软件包：
```
ox -r pm --install my_package.oxp
```
### 移除软件包
用选项"-r"移除软件包:
```
ox -r pm -r sdl
```
### 清理软件包
使用选项"--clean"检测本地可以被移除的软件包：
```
ox -r pm --clean
```
可移除软件包是指软件包中不包含可执行程序，且不被其他软件包依赖的那些包。
### 修复
如果之前软件安装或移除过程出现异常，可能导致本地缺失一部分需要的软件包。
使用选项"--repair"检测本地信息，发现缺失的软件包，并重新安装：
```
ox -r pm --repair
```
### 系统软件包
OX软件的运行除了依赖OX语言编写的软件包，有时还依赖一些系统库和系统程序。
比如软件包"pb"依赖"gcc", "gnu-gettext", "pkgconf"这些系统软件包。
这些软件包用户可以自己安装或通过系统自带的软件包管理程序进行管理，也可以使用"pm"安装OX打包好的系统软件。
注意如果使用"pm"安装系统软件，有可能和系统自带的软件包管理程序存在冲突。
一般在Linux系统下，推荐使用系统自带的软件包管理器关系系统软件，如ubuntu下的apt, fedora下的dnf等。
在Windows下，如果用户使用msys2安装和管理OX，推荐使用msys2的pacman管理系统软件。
如果用户在Windows下是单独安装OX环境，推荐用"pm"安装管理系统软件包。

如果用户希望安装软件包时自动安装依赖的系统软件包，在同步命令中加入"--sys"选项：
```
ox -r pm -s --sys pb
```
通过上面的命令，"pm"会自动安装"pb"依赖的"gcc", "gnu-gettext", "pkgconf"等系统软件包。
## 软件包构建
OX提供软件包构建工具"pb"，用于自动构建软件包。

### build.ox
用户要自己创建一个软件包，需要编写一个"build.ox"描述软件包的基本构建方法。
"pb"程序读入"build.ox"，根据描述的方法调用"gettext", "doc", "gcc"等工具，自动构建软件包。

我们来试着用"pb"创建一个软件包"test"。

首先我们创建一个"build.ox"文件内容如下：
```
{
    name: "test" // 软件包名称
    homepage: "HOMEPAGE" //这里添加软件包的主页
    maintainer: "MAINTAINER" //这里是软件包维护者名字
    //这里是软件包描述
    description: {
        //英文描述
        "en": ""
        //中文描述
        "zh": ""
    }
    version: "0.0.1" //版本信息
    //软件包依赖
    dependencies: {
        "std": "0.0.1" //标准库软件包,最低版本为0.0.1
    }
    //pb构建过程依赖的软件包
    development_dependencies: {
    }
    //软件包包含的可执行程序
    executables: [
    ]
    //软件包提供的可以被外部引用的库脚本
    libraries: [
    ]
}
```
然后我们在相同目录下创建一个可执行脚本"test.ox",内容如下：
```
ref "std/io"

stdout.puts(L"test!\n")
```
我们将脚本名添加到"build.ox"中：
```
{
    ...
    //软件包包含的可执行程序
    executables: [
        "test"
    ]
    ...
}
```
### 构建软件包
在相同目录下，运行以下命令，构建软件包"test":
```
ox -r pb
```
我们看到当前目录下生成了文件"package.ox"，表示构建已经完成。

我们还可以用运行以下命令，构建并打包软件包"test":
```
ox -r pb -p
```
可以看到当前目录下生成了软件包"test-all-0.0.1.oxp"。
这样我们就可以通过"pm"程序来安装这个软件包：
```
ox -r pm --install test-all-0.0.1.oxp
```
### 多语言
运行以下命令更新多语言信息：
```
ox -r pb --update-text
```
我们发现"pb"创建了信息模板文件"locale/test.pot"。

如果我们要支持中文，运行以下命令：
```
ox -r pb --gen-po zh_CN
```
"pb"调用"gettext"工具创建了中文翻译信息文件"locale/zh_CN.po"。

我们可以编辑"locale/zh_CN.po"将其中的文本信息翻译成中文：
```
#: test.ox:3
msgid "test!\n"
msgstr "测试!\n"
```
然后我们重新构建软件包：
```
ox -r pb -p
```
我们发现软件包中已经自动包含了中文信息翻译"locale/zh_CN/LC_MESSAGES/test.mo"文件。

我们重新安装软件包，通过设置环境变量为"LANG=zh_CN.UTF-8"或"LANG=en_US.UTF-8"，运行程序就可以切换中英文显示。
### 文档
我们可以在脚本中加入文档描述：
```
ref "std/io"

/*?
 *? @package test 测试.
 *? @exe 打印测试消息
 */
stdout.puts(L"test!\n")
```
重新构建软件包：
```
ox -r pb -p
```
我们发现软件包中包含了文档markdown文件"doc/md/test/test.md"和"doc/md/test/test_test.md"。
安装后，我们可以在OX安装目录的"share/ox/doc/md/test"子目录下查看这些文档。
### 原生模块
我们可以在软件包中增加原生模块的构建规则。
在"build.ox"中增加"oxn_modules"属性，在其中加入原生构建规则:
```
{
    ...
    "oxn_modules": {
        //原生模块"test_oxn.oxn"
        "test_oxn": {
            cflags: "" //GCC编译器选项
            libs: "" //链接选项
            //C源码文件
            sources: [
                "test.oxn.c"
            ]

        }
    }
    ...
}
```
重新构建软件包：
```
ox -r pb -p
```
我们发现"pb"调用gcc编译生成了原生模块"test_oxn.oxn"。
### oxngen
"pb"还可以调用"oxngen"扫描头文件，自动生成原生模块。
在"build.ox"中增加"oxngen_targets"属性，在其中加入"oxngen"相关规则：
```
{
    ...
    oxngen_targets: {
        //原生模块"test_oxngen.oxn"
        test_oxngen: {
            cflags: "" //GCC编译器选项
            libs: "" /链接选项
            input_files: [
                "test.h" //扫描"test.h"
            ]
        }
    }
    ...
}
```
重新构建软件包：
```
ox -r pb -p
```
我们发现"pb"调用"oxngen"和gcc生成了原生模块"test_oxngen.oxn"。
### 清理
"pb"在构建过程中会生成一些中间文件，再次构建时，如果"pb"发现中间文件已经存在且不需要更新，就会直接使用这些文件。

如果用户想清理这些中间文件，可以运行以下命令删除中间文件：
```
ox -r pb --clean
```
### 构建参数
有时我们需要在构建过程中增加一些选项，通过调整这些选项来调整构建方法。我们可以通过构建参数来实现。
在调用"pb"时，我们可以通过增加选项"-s"来增加一个构建参数。
参数的形式为"参数名=参数值"，如果不包含"=参数值"部分，则参数值设置为布尔值true，如果包含"=参数值"部分，则参数值为"="之后的字符串。
如:
```
ox -r pb -p -s DEBUG -s TAG=test
```
上面的命令定义了两个参数"DEBUG"和"TAG"。其中DEBUG的值为true，TAG的值为"test"。

在build.ox中我们可以通过以下方式访问这些参数：
```
config = argv[0] //配置被当作参数传递给"build.ox"，其中config.p为参数对象，其属性对应参数名，属性值为参数值。

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
这样我们可以通过参数DEBUG控制编译时是否使能调试信息，通过参数"TAG"修改宏定义"TAG"的值。