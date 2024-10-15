#!/usr/bin/ox

ref "std/dir_r"
ref "std/io"
ref "std/log"

log: Log("checkpo")

checkpo: func(path) {
	text = File.load_text(path)
	ln = 1

	empty_str = null
	for text.split("\n") as line {
		if line.length > 0 && line[line.length - 1] == "\n" {
			line = line.slice(0, line.length - 1)
		}

		if line ~ /#, fuzzy/ {
			stdout.puts("\"{path}\": {ln}: {line}\n")
		}

		if empty_str && !(line ~ /\s*".+"\s*/p) {
			stdout.puts("\"{path}\": {ln - 1}: {empty_str}\n")
		}

		if line ~ /msgstr\s+""/ {
			empty_str = line
		} else {
			empty_str = null
		}

		ln += 1
	}

	if empty_str {
		stdout.puts("\"{path}\": {ln - 1}: {empty_str}\n")
	}
}

#dir = DirR(".", DirR.DIR_SKIP)
for dir as file {
	if file ~ /\.po$/ {
		log.debug("find \"{file}\"")
		checkpo(file)
	}
}
