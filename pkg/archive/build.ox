ref "std/Shell"
ref "../../build/pinfo"

config = argv[0]

cflags: Shell.output("{config.pc} libarchive --cflags").trim()
libs: Shell.output("{config.pc} libarchive --libs").trim()

{
    name: "archive"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "Multi-format archive and compression library"
        zh: "支持多种格式的数据打包和压缩库"
    }
    version: "0.0.1"
    dependencies: get_deps("ox", "std", "libarchive")
    development_dependencies: get_deps("ox_devel", "pb", "libarchive_devel")
    libraries: [
        "archive"
        "archive_entry"
    ]
    oxngen_targets: {
        archive: {
            cflags
            libs
            input_files: [
                "archive.h"
            ]
            decl_filters: [
                "archive_errno"
                "archive_error_string"
                "archive_write_new"
                /archive_write_set_.+/p
                /archive_write_add_filter.*/p
                "archive_write_open_filename"
                "archive_write_close"
                "archive_write_free"
                "archive_write_data"
                "archive_write_header"
                "archive_read_new"
                /archive_read_support_.+/p
                "archive_read_open_filename"
                "archive_read_close"
                "archive_read_free"
                "archive_read_next_header"
                "archive_read_data"
            ]
            number_macros: [
                /ARCHIVE_FORMAT_.+/p
                /ARCHIVE_FILTER_.+/p
                "ARCHIVE_OK"
                "ARCHIVE_EOF"
                "ARCHIVE_RETRY"
                "ARCHIVE_WARN"
                "ARCHIVE_FAILED"
                "ARCHIVE_FATAL"
            ]
        }
        archive_entry: {
            cflags
            libs
            input_files: [
                "archive_entry.h"
            ]
            decl_filters: [
                "archive_entry"
                "archive_entry_new"
                "archive_entry_free"
                "archive_entry_set_pathname"
                "archive_entry_set_pathname_utf8"
                "archive_entry_set_size"
                "archive_entry_set_filetype"
                "archive_entry_set_perm"
                "archive_entry_set_mtime"
                "archive_entry_set_atime"
                "archive_entry_set_ctime"
                "archive_entry_set_ctime"
                "archive_entry_set_symlink"
                "archive_entry_set_symlink_type"
                "archive_entry_set_symlink_utf8"
                "archive_entry_pathname"
                "archive_entry_size"
                "archive_entry_filetype"
                "archive_entry_perm"
                "archive_entry_mtime"
                "archive_entry_atime"
                "archive_entry_ctime"
                "archive_entry_symlink"
                "archive_entry_symlink_utf8"
                "archive_entry_symlink_type"
            ]
            number_macros: [
                /AE_IF.+/p
                /AE_SYMLINK_TYPE_.+/p
            ]
        }
    }
}
