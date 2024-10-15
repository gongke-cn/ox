ref "std/io"
ref "std/fs"
ref "./curl"

/*?
 *? @package curl curl: network transfer library
 *? @lib Downloader error.
 */
public DownloaderError: class {
    /*?
     *? Initialize a downloader error.
     *? @param code {Number} The error code.
     *? @param url {String} The URL of the resource.
     */
    $init(code, url) {
        this.url = url
        this.code = code
        this.message = curl_easy_strerror(code)
    }

    /*?
     *? Convert the error to string.
     *? @return {String} The result string.
     */
    $to_str() {
        return "DownloaderError: {this.url}: {this.code}: {this.message}"
    }
}

/*?
 *? @callback DownloadCallback The download callback function.
 *? @param size {Number} The donwloaded data length in bytes.
 */

/*?
 *? Downloader
 */
public Downloader: class {
    /*?
     *? Initialize the CURL library.
     */
    static global_init() {
        curl_global_init(CURL_GLOBAL_DEFAULT)
    }

    /*?
     *? Cleanup the resource of CURL library.
     */
    static global_cleanup() {
       curl_global_cleanup()
    }

    /*?
     *? Initialize a downloader object.
     *? @param url {String} The URL of the object to be downloaded.
     *? @param cb {DownloadCallback} The callback function.
     */
    $init(url, cb) {
        this.#url = url
        this.#cb = cb
        this.#curl = curl_easy_init()
        curl_easy_setopt(this.#curl, CURLOPT_URL, url)
    }

    /*?
     *? Set the CURL option.
     *? @param type {Number} The option type.
     *? @param val The option value.
     */
    set_opt(type, val) {
        curl_easy_setopt(this.#curl, type, val)
    }

    /*?
     *? Get CURL information.
     *? @param type {Number} The information type.
     *? @param ptr {C:void*} The information pointer.
     */
    get_info(type, ptr) {
        curl_easy_getinfo(this.#curl, type, ptr)
    }

    /*?
     *? Get the CURL response code.
     *? @return {Number} The response code.
     */
    get_response_code() {
        code = Int32()
        
        curl_easy_getinfo(this.#curl, CURLINFO_RESPONSE_CODE, &code)

        return code.$to_num()
    }

    /*?
    *? Download the object and store it to a string.
    *? @return {String} The result string.
    */
    to_str() {
        sb = String.Builder()
        downloaded_bytes = 0

        write_fn: func(buf, size, num) {
            sb.append(buf.$to_str(0, num))
            @downloaded_bytes += num

            if this.#cb {
                this.#cb(downloaded_bytes)
            }

            return num
        }.to_c(C.func_type(Size, UInt8.pointer, Size, Size))

        curl_easy_setopt(this.#curl, CURLOPT_WRITEFUNCTION, write_fn)
        
        r = curl_easy_perform(this.#curl)
        if r != CURLE_OK {
            throw DownloaderError(r, this.#url)
        }

        return sb.$to_str()
    }

   /*?
    *? Convert the downloader to a string.
    *? @return {String} The result string.
    */
    $to_str() {
        return this.to_str()
    }

   /*?
    *? Download the object and store it to a file.
    *? @param fn {String} The filename.
    */
    to_file(fn) {
        #file = File(fn, "wb")
        downloaded_bytes = 0

        write_fn: func(buf, size, num) {
            file.write(buf, 0, num)
            @downloaded_bytes += num

            if this.#cb {
                this.#cb(downloaded_bytes)
            }

            return num
        }.to_c(C.func_type(Size, UInt8.pointer, Size, Size))

        curl_easy_setopt(this.#curl, CURLOPT_WRITEFUNCTION, write_fn)
        
        r = curl_easy_perform(this.#curl)
        if r != CURLE_OK {
            file.$close()
            unlink(fn)
            throw DownloaderError(r, this.#url)
        }
    }

   /*?
    *? Close the downloader.
    */
    $close() {
        if this.#curl {
            curl_easy_cleanup(this.#curl)
            this.#curl = null
        }
    }
}
