ref "./log"
ref "./tools"
ref "./config"

//Prepare the building environment
public prepare: func(desc) {
    if !desc.development_dependencies {
        return
    }

    args = Object.keys(desc.development_dependencies).$to_str(" ")
    if  args == "" {
        return
    }

    if config.sys {
        args = "--sys {args}"
    }

    cmd = "ox -r pm --prep {args}"

    shell(cmd)
}