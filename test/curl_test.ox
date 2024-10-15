ref "std/log"
ref "curl"

log: Log("curl")

curl_global_init(CURL_GLOBAL_DEFAULT)

curl = curl_easy_init()

r = curl_easy_setopt(curl, CURLOPT_URL, "https://gitee.com")
if r != CURLE_OK {
    log.debug("set: {CURLOPT_URL} {curl_easy_strerror(r)}")
}

write_fn: func(buf, size, num) {
    log.debug("recv: {buf.$to_str(0, num)}")
    return num
}.to_c(C.func_type(Size, UInt8.pointer, Size, Size))

curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_fn)

r = curl_easy_perform(curl)
if r != CURLE_OK {
    log.debug("result: {curl_easy_strerror(r)}")
}

curl_easy_cleanup(curl)

curl_global_cleanup()
