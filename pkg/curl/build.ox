ref "std/Shell"
ref "../../build/pinfo"

config = argv[0]

version: Shell.output("{config.pc} libcurl --modversion").trim()

{
    name: "curl"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "curl - network transfer library"
        zh: "curl - 网络传输库"
    }
    version: "0.0.1"
    dependencies: get_deps("ox", "std:0.0.1", "libcurl", "ca-certificates")
    development_dependencies: get_deps("ox_devel", "pb", "libcurl_devel")
    libraries: [
        "curl"
        "downloader"
    ]
    oxngen_targets: {
        curl: {
            cflags: Shell.output("{config.pc} libcurl --cflags").trim()
            libs: Shell.output("{config.pc} libcurl --libs").trim()
            input_files: [
                "curl/curl.h"
            ]
            references: {
                "std/socket": [
                    "sockaddr"
                ]
            }
            file_filters: [
                "curl/curl.h"
                "curl/easy.h"
            ]
            decl_filters: [
                "curl_global_init"
                "curl_global_cleanup"
                "curl_easy_init"
                "curl_easy_cleanup"
                "curl_easy_perform"
                "curl_easy_setopt"
                "curl_easy_getinfo"
                "curl_easy_strerror"
                /CURLE_.+/p
                /CURLOPT_.+/p
                /CURLINFO_.+/p
            ]
            rev_decl_filters: [
                "CURLOPT_HTTPPOST"
                "CURLOPT_PUT"
                "CURLOPT_PROGRESSFUNCTION"
                "CURLOPT_RANDOM_FILE"
                "CURLOPT_EGDSOCKET"
                "CURLOPT_DNS_USE_GLOBAL_CACHE"
                "CURLOPT_IOCTLFUNCTION"
                "CURLOPT_IOCTLDATA"
                "CURLOPT_CONV_FROM_NETWORK_FUNCTION"
                "CURLOPT_CONV_TO_NETWORK_FUNCTION"
                "CURLOPT_CONV_FROM_UTF8_FUNCTION"
                "CURLOPT_SOCKS5_GSSAPI_SERVICE"
                "CURLOPT_PROTOCOLS"
                "CURLOPT_REDIR_PROTOCOLS"
                "CURLOPT_SSL_ENABLE_NPN"
                "CURLINFO_SIZE_UPLOAD"
                "CURLINFO_SIZE_DOWNLOAD"
                "CURLINFO_SPEED_DOWNLOAD"
                "CURLINFO_SPEED_UPLOAD"
                "CURLINFO_CONTENT_LENGTH_DOWNLOAD"
                "CURLINFO_CONTENT_LENGTH_UPLOAD"
                "CURLINFO_LASTSOCKET"
                "CURLINFO_TLS_SESSION"
                "CURLINFO_PROTOCOL"
            ]
            number_macros: [
                "CURL_GLOBAL_DEFAULT"
            ]
        }
    }
}
