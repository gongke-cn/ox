#!/usr/bin/ox

ref "std/io" *
ref "std/log" *

log: Log("bytecode")

bytecodes = {
    "dup": "sd"
    "not": "sd"
    "to_num": "sd"
    "rev": "sd"
    "neg": "sd"
    "typeof": "sd"
    "not_null": "sd"
    "global": "s"
    "owned": "cs"
    "curr": "d"
    "get_ptr": "sd"
    "get_value": "sd"
    "set_value": "ss"
    "add": "ssd"
    "sub": "ssd"
    "match": "ssd"
    "exp": "ssd"
    "mul": "ssd"
    "div": "ssd"
    "mod": "ssd"
    "shl": "ssd"
    "shr": "ssd"
    "ushr": "ssd"
    "lt": "ssd"
    "gt": "ssd"
    "le": "ssd"
    "ge": "ssd"
    "instof": "ssd"
    "eq": "ssd"
    "ne": "ssd"
    "and": "ssd"
    "xor": "ssd"
    "or": "ssd"
    "load_null": "d"
    "load_true": "d"
    "load_false": "d"
    "this": "d"
    "this_b": "od"
    "argv": "d"
    "get_cv": "cd"
    "get_pp": "pd"
    "get_lt": "td"
    "get_ltt": "Td"
    "get_t": "id"
    "set_t": "is"
    "set_t_ac": "is"
    "get_t_b": "oid"
    "set_t_b": "ois"
    "set_t_b_ac": "ois"
    "get_n": "cd"
    "get_g": "cd"
    "get_r": "id"
    "get_p": "ssd"
    "lookup_p": "ssd"
    "set_p": "sss"
    "get_a": "id"
    "throw": "s"
    "ret": "s"
    "jmp": "l"
    "deep_jmp": "ol"
    "jt": "sl"
    "jf": "sl"
    "jnn": "sl"
    "str_start": "s"
    "str_start_t": "ss"
    "str_item": "s"
    "str_item_f": "cs"
    "str_end": "d"
    "call_start": "ss"
    "arg": "s"
    "arg_spread": "s"
    "call_end": "d"
    "call_end_tail": "d"
    "try_start": "ll"
    "try_end": ""
    "catch": "d"
    "catch_end": ""
    "finally": ""
    "sched": ""
    "sched_start": ""
    "yield": "sd"
    "s_pop": ""
    "iter_start": "s"
    "iter_step": "dl"
    "apat_start": "s"
    "apat_next": ""
    "apat_get": "d"
    "apat_rest": "d"
    "opat_start": "s"
    "opat_get": "sd"
    "opat_rest": "d"
    "a_new": "d"
    "a_start": "s"
    "a_next": ""
    "a_item": "s"
    "a_spread": "s"
    "a_end": ""
    "o_new": "d"
    "o_start": "s"
    "o_prop": "ss"
    "o_spread": "s"
    "p_start": ""
    "p_get": "d"
    "p_rest": "d"
    "f_new": "id"
    "c_new": "dd"
    "c_parent": "ss"
    "c_const": "sss"
    "c_var": "sss"
    "c_acce": "ssss"
    "c_ro_acce": "sss"
    "e_start": "s"
    "e_start_n": "cs"
    "e_item": "c"
    "b_start": "s"
    "b_start_n": "cs"
    "b_item": "c"
    "set_name": "ss"
    "set_name_g": "ss"
    "set_name_s": "ss"
    "set_scope": "ss"
    "name_nn": "cs"
    "prop_nn": "cs"
    "pprop_nn": "ps"
    "stub": "l"
    "nop": ""
}

push_bytecodes = [
    "str_start"
    "str_start_t"
    "call_start"
    "try_start"
    "sched_start"
    "iter_start"
    "apat_start"
    "opat_start"
    "a_start"
    "o_start"
    "p_start"
    "e_start"
    "e_start_n"
    "b_start"
    "b_start_n"
]

pop_bytecodes = [
    "str_end"
    "call_end"
    "call_end_tail"
    "finally"
    "s_pop"
    "a_end"
]

models: Set()
for Object.values(bytecodes) as m {
    models.add(m)
}

model_name: func(m) => if m {m} else {"noarg"}

gen_def: func() {
    model_struct: func(m) {
        if !m {
            return ""
        }

        id = 0;

        return ''
    struct {
        OX_GenCommand g;
{{m.$iter().map(func {
    r = "        int {$}{id};"
    @id += 1
    return r
}).$to_str("\n")}}
    } {{m}};''
    }

    stdout.puts(''
typedef enum {
{{Object.keys(bytecodes).map(("    OX_BC_{$}")).$to_str(",\n")}}
} OX_ByteCode;

typedef enum {
{{models.$iter().map(("    OX_BC_MODEL_{model_name($)}")).$to_str(",\n")}}
} OX_BcModel;

typedef struct {
    OX_ByteCode bc;
    OX_Location loc;
} OX_GenCommand;

typedef union {
    OX_ByteCode   bc;
    OX_GenCommand g;
{{models.$iter().map((model_struct($))).$to_str("\n")}}
} OX_Command;
    '')
}

gen_cmd: func() {
    cmd_func: func(m) {
        m_name = model_name(m)
        id = 0

        if m_name == "noarg" {
            return ''
static void
cmd_model_{{m_name}} (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty)
{
    add_cmd(ctxt, c, ty);
}
            ''
        } else {
            for m as c {
                args += ", int {c}{id}"
                code += "\n    cmd->{m}.{c}{id} = {c}{id};"
                id += 1
            }

            return ''
static void
cmd_model_{{m_name}} (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty{{args}})
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);
{{code}}
}
            ''
        }
    }

    bc_macro: func([bc, m]) {
        mn = model_name(m)
        id = 0

        for m as c {
            args += ", {c}{id.+=1}"
        }

        code = "        cmd_model_{mn}(ctxt, c, OX_BC_{bc}{args});\\"

        if bc == "stub" {
            code += "\n        assert(c->stack_level == ox_vector_item(&c->labels, l0).stack_level);\\"
        } elif push_bytecodes.has(bc) {
            code += "\n        c->stack_level ++;\\"
        } elif pop_bytecodes.has(bc) {
            code += "\n        c->stack_level --;\\"
        }
        
        return ''
#define cmd_{{bc}}(ctxt, c{{args}})\
    OX_STMT_BEGIN\
{{code}}
    OX_STMT_END
''
    }

    bc_len: func(m) {
        len = 1

        for m as c {
            case c {
                "i", "c", "p", "t", "T", "l" {
                    len += 2
                }
                * {
                    len += 1
                }
            }
        }

        return len
    }

    mark_regs: func(m) {
        id = 0
        for m as c {
            case c {
            "s", "d" {
                code += "\
        reg = &ox_vector_item(&c->regs, cmd->{model_name(m)}.{c}{id});
        reg->off = off;
"
            }
            }
            id += 1
        }
        return "\
    case OX_BC_MODEL_{model_name(m)}:
{code}\
        break;
"
    }

    alloc_regs: func(m) {
        id = 0
        for m as c {
            case c {
            "s", "d" {
                code += "\
        reg = &ox_vector_item(&c->regs, cmd->{model_name(m)}.{c}{id});
        if (reg->id == -1) \{
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        \}
"
            }
            }
            id += 1
        }
        return "\
    case OX_BC_MODEL_{model_name(m)}:
{code}\
        break;
"
    }

    bc_store: func(m) {
        id = 0
        for m as c {
            code += "        id = cmd->{model_name(m)}.{c}{id};\n"
            case c {
            "s", "d" {
                code += "\
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
"
            }
            "i", "c", "p", "t", "T" {
                code += "\
        u16 = id;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
"
            }
            "o" {
                code += "\
        u8 = id;
       *bc ++ = u8;
"
            }
            "l" {
                code += "\
        u16 = c->labels.items[id].off;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
"
            }
            }
            id += 1
        }

        return "\
    case OX_BC_MODEL_{model_name(m)}:
        *bc ++ = cmd->bc;
{code}\
        break;
"
    }

    bc_decompile: func(m) {
        id = 0
        for m as c {
            case c {
            "i", "l" {
                code += "\
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, \"%d \", u16);
"
            }
            "c" {
                code += "\
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, \"c%d(\", u16);
        dump_const(ctxt, s, u16, fp);
        fprintf(fp, \") \");
"
            }
            "p" {
                code += "\
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, \"p%d(\", u16);
        dump_private(ctxt, s, u16, fp);
        fprintf(fp, \") \");
"
            }
            "t" {
                code += "\
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, \"t%d(\", u16);
        dump_local_text(ctxt, s, u16, fp);
        fprintf(fp, \") \");
"
            }
            "T" {
                code += "\
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, \"t%d(\", u16);
        dump_local_templ(ctxt, s, u16, fp);
        fprintf(fp, \") \");
"
            }
            "s", "d" {
        code += "\
        u8 = *bc ++;
        fprintf(fp, \"r%d \", u8);
"
            }
            "o" {
                code += "\
        u8 = *bc ++;
        fprintf(fp, \"%d \", u8);
"
            }
            }
            ic += 1
        }

        return "\
    case OX_BC_MODEL_{model_name(m)}:
{code}\
        break;
"
    }

    stdout.puts("\
{models.$iter().map((cmd_func($))).$to_str("\n")}

{Object.entries(bytecodes).map((bc_macro($))).$to_str("\n")}

/*Bytecode name table.*/
static const char*
bytecode_names[] = \{
{Object.keys(bytecodes).map(("    \"{$}\"")).$to_str(",\n")}
\};

/*Bytecode -> model map table.*/
static const OX_BcModel
bytecode_models[] = \{
{Object.values(bytecodes).map(("    OX_BC_MODEL_{model_name($)}")).$to_str(",\n")}
\};

/*Bytecode model length table.*/
static const uint8_t
bytecode_len_table[] = \{
{models.$iter().map(("    {bc_len($)}")).$to_str(",\n")}
\};

/*Mark the registers lifetime used by the command.*/
static void
bytecode_mark_regs (OX_Context *ctxt, OX_Compiler *c, int off, OX_Command *cmd)
\{
    OX_BcModel m = bytecode_models[cmd->bc];
    OX_CompRegister *reg;

    switch (m) \{
{models.$iter().map((mark_regs($))).$to_str("")}
    \}
\}

/*Allocate registers used by the command.*/
static OX_Result
bytecode_alloc_regs (OX_Context *ctxt, OX_Compiler *c, int off, OX_Command *cmd)
\{
    OX_BcModel m = bytecode_models[cmd->bc];
    OX_CompRegister *reg;
    OX_Result r = OX_OK;

    switch (m) \{
{models.$iter().map((alloc_regs($))).$to_str("")}
    \}

    return r;
\}

/*Store the bytecode to buffer.*/
static void
bytecode_store (OX_Context *ctxt, OX_Compiler *c, OX_Command *cmd, uint8_t *bc)
\{
    OX_BcModel m = bytecode_models[cmd->bc];
    int id;
    uint8_t u8;
    uint16_t u16;

    switch (m) \{
{models.$iter().map((bc_store($))).$to_str("")}
    \}
\}

/*Decompile the bytecode.*/
static int
bytecode_decompile (OX_Context *ctxt, OX_BcScript *s, uint8_t *bc, FILE *fp)
\{
    OX_ByteCode t = *bc ++;
    OX_BcModel m = bytecode_models[t];
    uint8_t u8;
    uint16_t u16;

    fprintf(fp, \"%-10s \", bytecode_names[t]);

    switch (m) \{
{models.$iter().map((bc_decompile($))).$to_str("")}
    \}

    return bytecode_len_table[m];
\}
"   )
}

gen_run: func() {
    bc_run: func([bc, m]) {
        if bc == "stub" {
            return ""
        }

        params = ""
        args = ""
        code = ""
        id = 0
        off = 1

        for m as c {
            args += ", "

            case c {
            "i", "c", "p", "t", "T", "l" {
                params += "        uint16_t {c}{id};\n"
                code += "        {c}{id} = bc[{off}] << 8 | bc[{off + 1}];\n"
                off += 2;
            }
            * {
                params += "        uint8_t {c}{id};\n"
                code += "        {c}{id} = bc[{off}];\n"
                off += 1;
            }
            }

            case c {
            "s", "d" {
                args += "ox_values_item(ctxt, rs.regs, {c}{id})"
            }
            "c" {
                args += "&rs.s->cvs[{c}{id}]"
            }
            "p" {
                args += "&rs.s->pps[{c}{id}]"
            }
            * {
                args += "{c}{id}"
            }
            }

            id += 1
        }

        return "\
    case OX_BC_{bc}: \{
{params}\
{code}\
        r = do_{bc}(ctxt, &rs{args});
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += {off};
        break;
    \}
"
    }

    stdout.puts("\
{Object.entries(bytecodes).map((bc_run($))).$to_str("")}
"   )
}

case argv[1] {
    "-d" {
        gen_def()
    }
    "-c" {
        gen_cmd()
    }
    * {
        gen_run()
    }
}