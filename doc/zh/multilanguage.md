# 多语言
OX语言通过GNU gettext进行多语言支持。
## textdomain
OX语言中可以通过textdomain语句设置当前脚本绑定的textdomain:
```
textdomain "test" "/usr/share/locale"
```
上面的脚本设置当前脚本的domain名称为`"test"`, 对应的"mo"文件为"/usr/share/locale/%LOCALE%/LC_MESSAGES/test.mo"。
其中%LOCALE%对应当前的语言设置。

如果脚本中没有使用textdomain语句，则脚本的domain名称默认为当前软件包的名称。locale路径对应"%PACKAGE_DIR%/locale",
其中%PACKAGE_DIR%对应当前软件包的安装目录。
## 多语言支持
在程序中如果一个字符串我们希望通过其可以通过gettext进行本地化分析，我们在这个字符串前加入`L`标识：
```
stdout.puts(L"hello!\n")
```
这样我们就可以通过运行gettext工具进行扫描：
```
#扫描源文件收集多语言字符串生成信息模板
ox -r gettext -o hello.pot hello.ox
```
生成多语言模板文件"hello.pot"。然后我们就可以使用GNU gettext工具生成本地语言翻译：
```
#由信息模板生成本地信息文件
msginit -i hello.pot -o zh_CN.po -l zh_CN

#翻译本地信息文件
...

#由本地信息文件生成mo文件
msgfmt zh_CN.po -o locale/zh_CN/LC_MESSAGES/hello.mo
```
如果我们使用软件包构建工具"pb"构建软件包,我们也可以使用"pb"生成需要的"pot","po"和"mo"文件：
```
#生成pot文件
ox -r pb --update-text

#生成zh_CN.po文件
ox -r pb --gen-po zh_CN

#翻译本地信息文件
...

#生成mo文件
ox -r pb --update-text
```