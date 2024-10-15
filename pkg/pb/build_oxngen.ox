ref "std/fs"
ref "std/path"
ref "std/lang"
ref "std/io"
ref "./config"
ref "./tools"
ref "./log"

//Build a oxngen target.
build_target: func(oxn, def) {
    src = "{config.outdir}/{oxn}.oxn.c"
    if need_update(src, ["build.ox"]) {
        oxngen_flags = "--pc {config.pc}"

        if config.target_arch {
            oxngen_flags += " -t {config.target_arch}"
        }

        cmd = "{config.ox} -r oxngen {oxngen_flags} -i build.ox -o {config.outdir} {def.cflags}"
        shell(cmd)
    }

    obj_file = "{config.outdir}/{oxn}.o"
    dep_file = obj_file.replace(/\.o$/, ".d")
    if need_update(obj_file, [src, ...get_dep_files(dep_file)]) {
        mkdir_p(dirname(obj_file))
        cmd = "{config.cc} -o {obj_file} -c -fPIC {src} -Wall {def.cflags} {config.cflags} -MMD -MF {dep_file}"
        shell(cmd)
    }

    oxn_name = "{config.outdir}/{oxn}.oxn"
    if need_update(oxn_name, [obj_file]) {
        mkdir_p(dirname(oxn_name))

        if config.target == OX.target {
            ox_libs = OX.libs
        }

        cmd = "{config.cc} -o {oxn_name} -shared {obj_file} -lox {def.libs} {config.libs} {ox_libs}"
        shell(cmd)
    }
}

//Build the oxngen.
public build_oxngen: func(desc) {
    for Object.entries(desc) as [oxn, def] {
        build_target(oxn, def)
    }
}
