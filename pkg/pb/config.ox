//Command.
public Command: class {
    enum {
        HELP
        BUILD
        PACKAGE
        GEN_PO
        UPDATE_TEXT
        CLEAN
        PREPARE
    }
}

public config: {
    cmd: Command.BUILD
    cc: "gcc"
    pc: "pkg-config"
    input: "build.ox"
    p: {
    }
}