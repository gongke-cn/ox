ref "std/shell"
ref "../../build/pinfo"

config = argv[0]

cflags = Shell.output("{config.pc} openssl --cflags").trim()
libs = Shell.output("{config.pc} openssl --libs").trim()

{
    name: "ssl"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "OpenSSL - Secure Sockets Layer and cryptography libraries"
        zh: "OpenSSL - 安全套接字及加密库"
    }
    version: "0.0.1"
    dependencies: get_deps("ox", "std:0.0.1", "libssl", "libcrypto")
    development_dependencies: get_deps("ox_devel", "pb", "openssl_devel")
    libraries: [
        "ssl"
        "err"
    ]
    oxngen_targets: {
        ssl: {
            cflags
            libs
            references: {
                "std/io": [
                    "FILE"
                ]
            }
            input_files: [
                "openssl/ssl.h"
            ]
            number_macros: [
                "SSL_FILETYPE_ASN1"
                "SSL_FILETYPE_PEM"
            ]
            functions: {
                "SSL_load_error_strings": {
                    parameters: {
                    }
                    return: "int"
                }
                "SSL_library_init": {
                    parameters: {
                    }
                }
            }
            decl_filters: [
                "TLS_server_method"
                "SSL_load_error_strings"
                "SSL_library_init"
                "SSL_CTX_new"
                "SSL_CTX_free"
                "SSL_CTX_use_certificate_file"
                "SSL_CTX_use_PrivateKey_file"
                "SSL_new"
                "SSL_set_fd"
                "SSL_free"
                "SSL_accept"
                "SSL_get_error"
                "SSL_read"
                "SSL_write"
                "SSL_ERROR_WANT_READ"
                "SSL_ERROR_WANT_WRITE"
                "SSL_FILETYPE_ASN1"
                "SSL_FILETYPE_PEM"
            ]
        }
        err: {
            cflags
            libs
            input_files: [
                "openssl/err.h"
            ]
            decl_filters: [
                "ERR_get_error"
                "ERR_error_string_n"
            ]
        }
    }
}