ref "std/lang"
ref "std/io"
ref "json"
ref "../../build/pinfo"

config = argv[0]

if config.os == "windows" {
    socket_libs = "-lws2_32 -lwsock32"
}

if config.p.libs {
    libs = config.p.libs
} else {
    libs = OX.libs
}

{
    name: "std"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "OX standard librares"
        zh: "OX脚本语言标准库"
    }
    version: "0.0.2"
    dependencies: get_deps("ox")
    development_dependencies: get_deps("ox_devel", "pb")
    libraries: [
        "io"
        "time"
        "log"
        "path"
        "path_conv"
        "fs"
        "dir"
        "dir_r"
        "system"
        "lang"
        "math"
        "thread"
        "queue"
        "option"
        "rand"
        "char"
        "socket"
        "process"
        "temp_file"
        "text_util"
        "ast"
        "uri"
        "fiber"
        "shell"
        "copy"
    ]
    internal_libraries: [
        "ast_types"
    ]
    oxn_modules: {
        ast_types: {
            sources: [
                "ast_types.oxn.c"
            ]
        }
        log: {
            sources: [
                "log.oxn.c"
            ]
        }
        char: {
            sources: [
                "char.oxn.c"
            ]
        }
        dir: {
            sources: [
                "dir.oxn.c"
            ]
        }
        fs: {
            sources: [
                "fs.oxn.c"
            ]
        }
        io: {
            sources: [
                "io.oxn.c"
            ]
        }
        lang: {
            cflags: "-DTARGET=\"\\\"{config.target}\\\"\" -DLIBS=\"\\\"{libs}\\\"\" -DVERSION=\"\\\"{get_version("ox")}\\\"\""
            sources: [
                "lang.oxn.c"
            ]
        }
        math: {
            sources: [
                "math.oxn.c"
            ]
        }
        path: {
            sources: [
                "path.oxn.c"
            ]
        }
        process: {
            sources: [
                "process.oxn.c"
            ]
        }
        queue: {
            sources: [
                "queue.oxn.c"
            ]
        }
        rand: {
            sources: [
                "rand.oxn.c"
            ]
        }
        socket: {
            libs: "{socket_libs}"
            sources: [
                "socket.oxn.c"
            ]
        }
        system: {
            sources: [
                "system.oxn.c"
            ]
        }
        thread: {
            sources: [
                "thread.oxn.c"
            ]
        }
        time: {
            sources: [
                "time.oxn.c"
            ]
        }
        fiber: {
            sources: [
                "fiber.oxn.c"
            ]
        }
    }
}
