ref "std/system"
ref "std/io"
ref "std/path"
ref "std/fs"
ref "std/lang"
ref "./config"
ref "./tools"
ref "./log"

//Get the object file name.
public object_name: func(oxn, src) {
    m = src.match(/(.+)\.(c|cc|cxx|cpp|c\+\+)$/p)
    if m {
        return "{config.outdir}/{oxn}-{m.groups[1]}.o"
    } else {
        return "{config.outdir}/{oxn}-{src}.o"
    }
}

//Build an object file.
build_object: func(oxn, def, src) {
    src_st = Path(src)
    if !src_st.exist {
        throw NullError(L"cannot find file \"{src}\"")
    }

    obj_name = object_name(oxn, src)
    dep_name = obj_name.replace(/\.o$/, ".d")

    if need_update(obj_name, [src, ...get_dep_files(dep_name)]) {
        mkdir_p(dirname(obj_name))
        cmd = "{config.cc} -o {obj_name} -c -fPIC -Wall {def.cflags} {config.cflags} {src} -MMD -MF {dep_name}"
        shell(cmd)
    }
}

//Build an OXN module.
build_module: func(oxn, def) {
    for def.sources as src {
        build_object(oxn, def, src)
    }

    obj_files = def.sources.$iter().map((object_name(oxn, $))).to_array()
    oxn_name = "{config.outdir}/{oxn}.oxn"

    if need_update(oxn_name, obj_files) {
        mkdir_p(dirname(oxn_name))

        if config.target == OX.target {
            ox_libs = OX.libs
        }

        cmd = "{config.cc} -o {oxn_name} -shared {obj_files.$to_str(" ")} -lox {def.libs} {config.libs} {ox_libs}"
        shell(cmd)
    }
}

//Build the OXN modules.
public build_oxn: func(desc) {
    for Object.entries(desc) as [oxn, def] {
        build_module(oxn, def)
    }
}
