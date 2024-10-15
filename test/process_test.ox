#!/usr/bin/ox

ref "std/process"
ref "std/log"

log: Log("process_test")

cmd = "ox --help"
p = Process(cmd, Process.STDOUT)
log.debug("cmd: {cmd}")

while !p.stdout.eof {
    line = p.stdout.gets()
    if line != null {
        log.debug("stdout: {line}")
    }
}

s = p.wait()
log.debug("status: {s}")
