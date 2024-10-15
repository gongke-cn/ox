#!/usr/bin/ox

ref "std/log"
ref "std/option"
ref "std/io"
ref "std/fs"
ref "std/path"
ref "std/path_conv"
ref "std/system"
ref "std/shell"
ref "std/copy"
ref "std/dir"
ref "std/dir_r"
ref "std/copy"
ref "std/temp_file"
ref "json"

log: Log("wininst")

config = {
    outdir: "o-msys"
    files: []
}

options: Option([
    {
        short: "v"
        arg: Option.STRING
        help: "Set the Ox version"
        on_option: func(opt, arg) {
            config.version = arg
        }
    }
    {
        short: "o"
        arg: Option.STRING
        help: "Set the output directory"
        on_option: func(opt, arg) {
            config.outdir = arg
        }
    }
])

if !options.parse(argv) {
    return 1
}

out = "{config.outdir}/wininst"
#outdir = TempDir(out)
outdir.mkdir_p()

cmd = "ox -r pm -I {out} -S {config.outdir}/oxp -s --sys ox pm"
Shell.run(cmd, Shell.OUTPUT|Shell.ERROR)

copy_to_out: func(src, dst) {
    mkdir_p(dirname(dst))
    copy(src, dst)
}

//Add wrapper exe.
copy_to_out("{config.outdir}/bin/ox.exe", "{out}/bin/ox.exe")

//Add files.
for DirR(out, DirR.DIR_SKIP) as file {
    relpath = file.replace("{out}/", "", 0, true)
    config.files.push(relpath)
}

//Build delete jobs.
root = {}
for config.files as file {
    n = root
    for file.split("/") as seg {
        sub = n[seg]
        if !sub {
            sub = {}
            n[seg] = sub
        }

        n = sub
    }
}

//log.debug(JSON.to_str(root, "  "))

del_sb = String.Builder()
del_file: func(n, fn) {
    if Object.keys(n).to_array().length {
        for Object.entries(n) as [k, v] {
            if fn {
                sub_fn = "{fn}\\{k}"
            } else {
                sub_fn = k
            }

            del_file(v, sub_fn)
        }

        if fn {
            del_sb.append("    RMDir $INSTDIR\\{fn}\n")
        }
    } elif fn {
        del_sb.append("    Delete $INSTDIR\\{fn}\n")
    }
}
del_file(root)

inst_exe = "ox-windows-{config.version}-installer.exe"

File.store_text("{out}/wininst.nsi", ''
OutFile "{{inst_exe}}"
InstallDir "C:\ox"
Caption "OX Installer"
UninstallCaption "OX Uninstaller"
DirText "Select the installation directory"
Page directory 
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles
RequestExecutionLevel none

Function GetPath
    ReadRegStr $0 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH"
    StrLen $1 $0
    StrCpy $2 ""
    StrCpy $3 0
    StrCpy $4 0
Loop:
    IntCmp $4 $1 Find Continue LoopEnd
Continue:
    StrCpy $5 $0 1 $4
    StrCmp $5 ";" Find Next
Find:
    IntOp $6 $4 - $3
    StrCpy $5 $0 $6 $3
    StrCmp $5 "$INSTDIR\bin" Move
    StrCmp $5 "$INSTDIR\lib\ox\lib" Move
    IntCmp $6 0 Move
    StrCmp $2 "" First NotFirst
First:
    StrCpy $2 $5
    Goto Move
NotFirst:
    StrCpy $2 "$2;$5"
Move:
    StrCpy $3 $4
    IntOp $3 $3 + 1
Next:
    IntOp $4 $4 + 1
    Goto Loop
LoopEnd:
    StrCpy $0 $2
FunctionEnd

Function un.GetPath
    ReadRegStr $0 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH"
    StrLen $1 $0
    StrCpy $2 ""
    StrCpy $3 0
    StrCpy $4 0
Loop:
    IntCmp $4 $1 Find Continue LoopEnd
Continue:
    StrCpy $5 $0 1 $4
    StrCmp $5 ";" Find Next
Find:
    IntOp $6 $4 - $3
    StrCpy $5 $0 $6 $3
    StrCmp $5 "$INSTDIR\bin" Move
    StrCmp $5 "$INSTDIR\lib\ox\lib" Move
    IntCmp $6 0 Move
    StrCmp $2 "" First NotFirst
First:
    StrCpy $2 $5
    Goto Move
NotFirst:
    StrCpy $2 "$2;$5"
Move:
    StrCpy $3 $4
    IntOp $3 $3 + 1
Next:
    IntOp $4 $4 + 1
    Goto Loop
LoopEnd:
    StrCpy $0 $2
FunctionEnd

Section

{{config.files.$iter().map(func(file) {
    dir = dirname(file).replace("/", "\\")
    fn = file.replace("/", "\\")

    return ''
    SetOutPath $INSTDIR\{{dir}}
    File {{fn}}

    ''
}).$to_str("\n")}}
    WriteUninstaller "$INSTDIR\uninstaller.exe"

    Call GetPath
    Pop $0
    StrCmp $0 "" Empty NotEmpty
Empty:
    StrCpy $0 "$INSTDIR\bin;$INSTDIR\lib\ox\lib"
    Goto PathEnd
NotEmpty:
    StrCpy $0 "$0;$INSTDIR\bin;$INSTDIR\lib\ox\lib"
PathEnd:
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" $0

    WriteRegStr HKCR ".ox" "" "OxScript"
    WriteRegStr HKCR "OxScript" "" "OX script"
    WriteRegExpandStr HKCR "OxScript\shell\open\command" "" "$\"$INSTDIR\bin\ox-win.exe$\" $\"%1$\""
    WriteRegStr HKCR "OxScript\DefaultIcon" "" "$INSTDIR\bin\ox-win.exe,0"
    WriteRegStr HKCR ".oxn" "" "OxNativeModule"
    WriteRegStr HKCR "OxNativeModule" "" "OX script native module"
    WriteRegExpandStr HKCR "OxNativeModule\shell\open\command" "" "$\"$INSTDIR\bin\ox-win.exe$\" $\"%1$\""
    WriteRegStr HKCR "OxNativeModule\DefaultIcon" "" "$INSTDIR\bin\ox-win.exe,1"
    WriteRegStr HKCR ".oxp" "" "OxPackage"
    WriteRegStr HKCR "OxPackage" "" "OX script package"
    WriteRegStr HKCR "OxPackage\DefaultIcon" "" "$INSTDIR\bin\ox-win.exe,2"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OX" "DisplayName" "OX script language"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OX" "Publisher" "Gong Ke"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OX" "DisplayVersion" "{{config.version}}"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OX" "DisplayIcon" "$\"$INSTDIR\bin\ox-win.exe$\""
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OX" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OX" "UninstallString" "$\"$INSTDIR\uninstaller.exe$\""
SectionEnd

Section "uninstall"
{{del_sb.$to_str()}}
    Delete $INSTDIR\uninstaller.exe
    RMDir $INSTDIR

    Call un.GetPath
    Pop $0
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" $0

    DeleteRegKey HKCR ".ox"
    DeleteRegKey HKCR "OxScript"
    DeleteRegKey HKCR ".oxn"
    DeleteRegKey HKCR "OxNativeModule"
    DeleteRegKey HKCR ".oxp"
    DeleteRegKey HKCR "OxPackage"
    DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OX"
SectionEnd

''
)

cmd = "makensis {out}/wininst.nsi"
if Shell.run(cmd) != 0 {
    throw SystemError("run \"{cmd}\" failed")
}

copy("{out}/{inst_exe}", "{config.outdir}/{inst_exe}")

return 0
