# Comments
Comments in the OX language are mainly used to add descriptive and explanatory text, and they are directly ignored by the OX interpreter during script execution.
The OX language adopts comment forms similar to those in the C language.
## Single-line Comments
A single-line comment starts with "//" and ends at the end of the current line.
```
a = 1 //Comments of 1st line
b = 2 //Comments of 2nd line
```

Syntax description:
```
single_line_comment: "//" single_line_comment_character*

single_line_comment: All characters except LF line break
```
## Multi-line Comments
A multi-line comment starts with "/*" and ends with "*/", and can span multiple lines.
```
a = 1 /*one line comment*/
b = 1 /*multi-line comment
 multi-line comment
 multi-line comment*/
c = 1
```

Syntax description:
```
multi_line_comment: "/*" multi_line_comment_character* "*/"

multi_line_comment_character: All characters
```

## Shebang/Hashbang
A Shebang/Hashbang marker can also be added at the beginning of the script to indicate the interpreter program used by the script.
Linux/Unix systems load the specified executable program according to this marker to parse and execute the script.
The Shebang/Hashbang marker starts with "#!" and ends at the end of the line.
```
#!/usr/bin/ox

a = 1
b = 2
...
```
Note that the Shebang/Hashbang marker can only appear at the very beginning of the script and can only appear once.

Syntax description:
```
shebang: "#!" single_line_comment_character*
```