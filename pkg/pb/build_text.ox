ref "std/path"
ref "std/dir"
ref "std/io"
ref "std/system"
ref "std/fs"
ref "./config"
ref "./tools"
ref "./log"

//Build the mo file.
build_mo: func(desc, po) {
    src = "locale/{po}"
    m = po.match(/(.+)\.po/ip)
    lang = m.groups[1]
    dir = "{config.outdir}/locale/{lang}/LC_MESSAGES"
    dst = "{dir}/{desc.name}.mo"

    mkdir_p(dir)

    if need_update(dst, [src]) {
        cmd = "msgfmt {src} -o {dst}"
        shell(cmd)
    }
}

//Build the text.
public build_text: func(desc) {
    path = "locale"

    langs = []

    if Path(path).exist {
        #dir = Dir(path)
        for dir as dent {
            if dent ~ /\.po$/i {
                langs.push(dent)
            }
        }
    }

    if langs.length {
        for langs as lang {
            build_mo(desc, lang)
        }
    }
}

//Update pot file.
update_pot: func(desc) {
    srcs = lookup_sources(desc, true)

    //Generate POT file.
    dst = "locale/{desc.name}.pot"

    if need_update(dst, srcs) {
        cmd = "{config.ox} -r gettext -o {dst} {srcs.$to_str(" ")}"
        shell(cmd)
    }
}

//Update text files.
public update_text: func(desc) {
    path = "locale"
    mkdir_p(path)

    //Update POT file.
    update_pot(desc)

    //Update PO file.
    if Path(path).exist {
        pot = "{path}/{desc.name}.pot"
        #dir = Dir(path)
        for dir as dent {
            if dent ~ /\.po$/i {
                po = "{path}/{dent}"
                if need_update(po, [pot]) {
                    cmd = "msgmerge -U {po} {pot}"
                    shell(cmd)
                }
            }
        }
    }
}

//Generate po file.
public gen_po: func(desc) {
    path = "locale"
    mkdir_p(path)

    //Update POT file.
    update_pot(desc)

    //Generate MO file.
    src = "locale/{desc.name}.pot"
    dst = "locale/{config.lang}.po"
    if Path(dst).exist {
        cmd = "msgmerge -U {dst} {src}"
        shell(cmd)
    } else {
        cmd = "msginit -i {src} -o {dst} -l {config.lang}"
        shell(cmd)
    }
}
