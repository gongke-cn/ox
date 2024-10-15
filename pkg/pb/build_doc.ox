ref "std/fs"
ref "std/io"
ref "./tools"
ref "./config"
ref "./log"

//Build the document.
public build_doc: func(desc) {
    srcs = lookup_sources(desc)
    if srcs.length == 0 {
        return
    }

    doc_dir = "{config.outdir}/doc/md/{desc.name}"
    doc_file = "{doc_dir}/{desc.name}.md"

    if need_update(doc_file, srcs) {
        mkdir_p(doc_dir)
        cmd = "{config.ox} -r doc -o {doc_dir} {srcs.$to_str(" ")}"
        shell(cmd)
    }
}
