# 注释
注释在OX语言中主要用于添加一些描述说明性文字，在脚本执行过程中注释被OX解释器直接忽略。
OX语言注释采用和C语言类似的注释形式。
## 单行注释
单行注释以“//”开始，直到当前行末尾结束。
```
a = 1 //此处为第一行注释
b = 2 //此处为第二行注释
```

语法描述:
```
single_line_comment: "//" single_line_comment_character*

single_line_comment: 除LF换行以外全部字符
```
## 多行注释
多行注释以"/*"开始，以"*/"结束，可以跨越多行。
```
a = 1 /*一行注释*/
b = 1 /*多行注释
 多行注释
 多行注释*/
c = 1
```
语法描述:
```
multi_line_comment: "/*" multi_line_comment_character* "*/"

multi_line_comment_character: 全部字符
```

## Shebang/Hashbang
在脚本的开始位置还可以加入Shebang/Hashbang标记，表示脚本使用的解释程序。
Linux/Unix系统会根据这个标记加载指定的可执行程序解析执行这个脚本。
Shebang/Hashbang标记以"#!"开始，直到行末尾结束。
```
#!/usr/bin/ox

a = 1
b = 2
...
```
注意Shebang/Hashbang标记只能出现在脚本最开始的位置，且只能出现一次。

语法描述:
```
shebang: "#!" single_line_comment_character*
```