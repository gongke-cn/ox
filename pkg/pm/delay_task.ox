ref "std/shell"
ref "std/io"
ref "std/fs"
ref "std/path"
ref "std/dir_r"
ref "std/path_conv"
ref "std/lang"
ref "./log"
ref "./config"

pl_need_delay = ["pm", "ox", "std", "archive", "oxp", "json", "curl"]

rm_sb = String.Builder()
pre_uninst_sb = String.Builder()
post_uninst_sb = String.Builder()
pre_inst_sb = String.Builder()
post_inst_sb = String.Builder()
need_install = false

//Delay task.
public DelayTask: class {
    //The root directory of the delay task.
    static rootdir {
        return "{config.install_dir}/share/ox/delay_task_root"
    }

    //Check if the package need  delay task.
    static need_delay(pn) {
        if fullpath(OX.install_dir) != fullpath(config.install_dir) {
            return false
        }
        
        return pl_need_delay.has(pn)
    }

    //Add pre-uninstall command.
    static pre_uninst(cmd) {
        pre_uninst_sb.append("{cmd}\n")
    }

    //Add post-uninstall command.
    static post_uninst(cmd) {
        post_uninst_sb.append("{cmd}\n")
    }

    //Add pre-install command.
    static pre_inst(cmd) {
        pre_inst_sb.append("{cmd}\n")
    }

    //Add post-install command.
    static post_inst(cmd) {
        post_inst_sb.append("{cmd}\n")
    }

    //Add unlink file operation.
    static unlink(file) {
        if config.os == "windows" {
            file = win_path(file)
            rm_sb.append("del \"{file}\"\n")
        } else {
            rm_sb.append("rm \"{file}\"\n")
        }
    }

    //Add remove directory operation.
    static rmdir(dir) {
        if config.os == "windows" {
            dir = win_path(dir)
            rm_sb.append("rd /S /Q \"{dir}\"\n")
        } else {
            rm_sb.append("rm -rf \"{dir}\"\n")
        }
    }

    //Add install package operation.
    static install(pn) {
        if !need_install {
            dir = DelayTask.rootdir

            log.debug("clear \"{dir}\"")

            if Path(dir).exist {
                DirR.rmdir(dir)
            }
            mkdir_p(dir)

            @need_install = true
        }
    }

    //Start delay task.
    static start() {
        rm = rm_sb.$to_str()
        pre_uninst = pre_uninst_sb.$to_str()
        post_uninst = post_uninst_sb.$to_str()
        pre_inst = pre_inst_sb.$to_str()
        post_inst = post_inst_sb.$to_str()

        if !rm.length &&
                !pre_uninst.length &&
                !post_uninst.length &&
                !pre_inst.length &&
                !post_inst.length &&
                !need_install {
            return
        }

        bat = "{config.install_dir}/share/ox/delay_task.bat"

        if config.os == "windows" {
            srcdir = win_path(DelayTask.rootdir)
            dstdir = win_path(config.install_dir)

            File.store_text(bat, ''
@echo off
{{pre_uninst}}
{{rm}}
{{post_uninst}}
{{pre_inst}}
if exist "{{srcdir}}" (
    xcopy "{{srcdir}}" "{{dstdir}}" /E /I /Y /Q > nul
    rd /S /Q "{{srcdir}}"
)
{{post_uninst}}

            '')
        } else {
            srcdir = DelayTask.rootdir
            dstdir = config.install_dir

            File.store_text(bat, ''
#!/bin/bash
{{pre_uninst}}
{{rm}}
{{post_uninst}}
{{pre_inst}}
if [ -d "{{srcdir}}" ]; then
    cp -a {{srcdir}}/* "{{dstdir}}"
    rm -rf "{{srcdir}}"
fi
{{post_uninst}}

            '')
            chmod(bat, 0o755)
        }
    }
}
