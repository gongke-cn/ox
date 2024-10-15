# Multilingual
OX language supports multilingualism through GNU gettext.
## textdomain
In OX language, the textdomain statement can be used to set the textdomain bound to the current script:
```
textdomain "test" "/usr/share/locale"
```
The above script sets the domain name of the current script to `"test"`, and the corresponding "mo" file is "/usr/share/locale/%LOCALE%/LC_MESSAGES/test.mo".
Among them, %LOCALE% corresponds to the current language setting.

If the textdomain statement is not used in the script, the domain name of the script defaults to the name of the current software package. The locale path corresponds to "%PACKAGE_DIR%/locale",
where %PACKAGE_DIR% corresponds to the installation directory of the current software package.
## Multilingual support
In the program, if we want a string to be analyzed for localization through gettext, we add the `L` identifier before this string:
```
stdout.puts(L"hello!\n")
```
In this way, we can scan by running the gettext tool:
```
#Scan source files to collect multilingual strings and generate information templates
ox -r gettext -o hello.pot hello.ox
```
Generate the multilingual template file "hello.pot". Then we can use the GNU gettext tool to generate local language translations:
```
#Generate local information files from information templates
msginit -i hello.pot -o zh_CN.po -l zh_CN

#Translate local information files
...

#Generate mo files from local information files
msgfmt zh_CN.po -o locale/zh_CN/LC_MESSAGES/hello.mo
```
If we use the software package build tool "pb" to build the software package, we can also use "pb" to generate the required "pot", "po" and "mo" files:
```
#Generate pot files
ox -r pb --update-text

#Generate zh_CN.po files
ox -r pb --gen-po zh_CN

#Translate local information files
...

#Generate mo files
ox -r pb --update-text
```