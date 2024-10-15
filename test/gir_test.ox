ref "std/log"
ref "json"
ref "gir"

log: Log("gir")

repo = GIRepository()
//repo.prepend_search_path("C:/ox/lib/girepository-1.0")

err = GError("test", 123, "just a test")

log.debug(err)

for repo.get_search_path() as path {
    log.debug("search path: {path}")
}

for repo.get_library_path() as path {
    log.debug("library path: {path}")
}

for repo.enumerate_versions("Gtk") as v {
    log.debug("version: {v}")
}

Gtk = repo.require("Gtk", "4.0")
Gio = repo.require("Gio", "2.0")

app = Gtk.Application.new("gir.ox", Gio.ApplicationFlags.default_flags).{
    $connect("activate", owned func(app) {
        win = Gtk.ApplicationWindow.new(app).{
            set_title("Window")
            set_default_size(200, 200)
            set_child(Gtk.Box.new(Gtk.Orientation.vertical, 1).{
                append(image = Gtk.Image.new().{
                    set_hexpand(true)
                    set_vexpand(true)
                })
                append(Gtk.Button.new_with_label("Hello").{
                    $connect("clicked", owned func() {
                        log.debug("hello clicked")
                        owned Gtk.FileDialog.new().{
                            open(win, null, owned func(dlg, res) {
                                try {
                                    file = dlg.open_finish(res)
                                    log.debug("file: {file.get_path()}")
                                    image.set_from_file(file.get_path())
                                    file.unref()
                                } catch e {
                                }
                            })
                        }
                    })
                })
            })
            present()
        }
    })
    run(0, null)
    unref()
}
