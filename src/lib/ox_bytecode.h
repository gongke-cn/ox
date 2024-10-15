typedef enum {
    OX_BC_dup,
    OX_BC_not,
    OX_BC_to_num,
    OX_BC_rev,
    OX_BC_neg,
    OX_BC_typeof,
    OX_BC_not_null,
    OX_BC_global,
    OX_BC_owned,
    OX_BC_curr,
    OX_BC_get_ptr,
    OX_BC_get_value,
    OX_BC_set_value,
    OX_BC_add,
    OX_BC_sub,
    OX_BC_match,
    OX_BC_exp,
    OX_BC_mul,
    OX_BC_div,
    OX_BC_mod,
    OX_BC_shl,
    OX_BC_shr,
    OX_BC_ushr,
    OX_BC_lt,
    OX_BC_gt,
    OX_BC_le,
    OX_BC_ge,
    OX_BC_instof,
    OX_BC_eq,
    OX_BC_ne,
    OX_BC_and,
    OX_BC_xor,
    OX_BC_or,
    OX_BC_load_null,
    OX_BC_load_true,
    OX_BC_load_false,
    OX_BC_this,
    OX_BC_this_b,
    OX_BC_argv,
    OX_BC_get_cv,
    OX_BC_get_pp,
    OX_BC_get_lt,
    OX_BC_get_ltt,
    OX_BC_get_t,
    OX_BC_set_t,
    OX_BC_set_t_ac,
    OX_BC_get_t_b,
    OX_BC_set_t_b,
    OX_BC_set_t_b_ac,
    OX_BC_get_n,
    OX_BC_get_g,
    OX_BC_get_r,
    OX_BC_get_p,
    OX_BC_lookup_p,
    OX_BC_set_p,
    OX_BC_get_a,
    OX_BC_throw,
    OX_BC_ret,
    OX_BC_jmp,
    OX_BC_deep_jmp,
    OX_BC_jt,
    OX_BC_jf,
    OX_BC_jnn,
    OX_BC_str_start,
    OX_BC_str_start_t,
    OX_BC_str_item,
    OX_BC_str_item_f,
    OX_BC_str_end,
    OX_BC_call_start,
    OX_BC_arg,
    OX_BC_arg_spread,
    OX_BC_call_end,
    OX_BC_call_end_tail,
    OX_BC_try_start,
    OX_BC_try_end,
    OX_BC_catch,
    OX_BC_catch_end,
    OX_BC_finally,
    OX_BC_sched,
    OX_BC_sched_start,
    OX_BC_yield,
    OX_BC_s_pop,
    OX_BC_iter_start,
    OX_BC_iter_step,
    OX_BC_apat_start,
    OX_BC_apat_next,
    OX_BC_apat_get,
    OX_BC_apat_rest,
    OX_BC_opat_start,
    OX_BC_opat_get,
    OX_BC_opat_rest,
    OX_BC_a_new,
    OX_BC_a_start,
    OX_BC_a_next,
    OX_BC_a_item,
    OX_BC_a_spread,
    OX_BC_a_end,
    OX_BC_o_new,
    OX_BC_o_start,
    OX_BC_o_prop,
    OX_BC_o_spread,
    OX_BC_p_start,
    OX_BC_p_get,
    OX_BC_p_rest,
    OX_BC_f_new,
    OX_BC_c_new,
    OX_BC_c_parent,
    OX_BC_c_const,
    OX_BC_c_var,
    OX_BC_c_acce,
    OX_BC_c_ro_acce,
    OX_BC_e_start,
    OX_BC_e_start_n,
    OX_BC_e_item,
    OX_BC_b_start,
    OX_BC_b_start_n,
    OX_BC_b_item,
    OX_BC_set_name,
    OX_BC_set_name_g,
    OX_BC_set_name_s,
    OX_BC_set_scope,
    OX_BC_name_nn,
    OX_BC_prop_nn,
    OX_BC_pprop_nn,
    OX_BC_stub,
    OX_BC_nop
} OX_ByteCode;

typedef enum {
    OX_BC_MODEL_sd,
    OX_BC_MODEL_s,
    OX_BC_MODEL_cs,
    OX_BC_MODEL_d,
    OX_BC_MODEL_ss,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_od,
    OX_BC_MODEL_cd,
    OX_BC_MODEL_pd,
    OX_BC_MODEL_td,
    OX_BC_MODEL_Td,
    OX_BC_MODEL_id,
    OX_BC_MODEL_is,
    OX_BC_MODEL_oid,
    OX_BC_MODEL_ois,
    OX_BC_MODEL_sss,
    OX_BC_MODEL_l,
    OX_BC_MODEL_ol,
    OX_BC_MODEL_sl,
    OX_BC_MODEL_ll,
    OX_BC_MODEL_noarg,
    OX_BC_MODEL_dl,
    OX_BC_MODEL_dd,
    OX_BC_MODEL_ssss,
    OX_BC_MODEL_c,
    OX_BC_MODEL_ps
} OX_BcModel;

typedef struct {
    OX_ByteCode bc;
    OX_Location loc;
} OX_GenCommand;

typedef union {
    OX_ByteCode   bc;
    OX_GenCommand g;
    struct {
        OX_GenCommand g;
        int s0;
        int d1;
    } sd;
    struct {
        OX_GenCommand g;
        int s0;
    } s;
    struct {
        OX_GenCommand g;
        int c0;
        int s1;
    } cs;
    struct {
        OX_GenCommand g;
        int d0;
    } d;
    struct {
        OX_GenCommand g;
        int s0;
        int s1;
    } ss;
    struct {
        OX_GenCommand g;
        int s0;
        int s1;
        int d2;
    } ssd;
    struct {
        OX_GenCommand g;
        int o0;
        int d1;
    } od;
    struct {
        OX_GenCommand g;
        int c0;
        int d1;
    } cd;
    struct {
        OX_GenCommand g;
        int p0;
        int d1;
    } pd;
    struct {
        OX_GenCommand g;
        int t0;
        int d1;
    } td;
    struct {
        OX_GenCommand g;
        int T0;
        int d1;
    } Td;
    struct {
        OX_GenCommand g;
        int i0;
        int d1;
    } id;
    struct {
        OX_GenCommand g;
        int i0;
        int s1;
    } is;
    struct {
        OX_GenCommand g;
        int o0;
        int i1;
        int d2;
    } oid;
    struct {
        OX_GenCommand g;
        int o0;
        int i1;
        int s2;
    } ois;
    struct {
        OX_GenCommand g;
        int s0;
        int s1;
        int s2;
    } sss;
    struct {
        OX_GenCommand g;
        int l0;
    } l;
    struct {
        OX_GenCommand g;
        int o0;
        int l1;
    } ol;
    struct {
        OX_GenCommand g;
        int s0;
        int l1;
    } sl;
    struct {
        OX_GenCommand g;
        int l0;
        int l1;
    } ll;

    struct {
        OX_GenCommand g;
        int d0;
        int l1;
    } dl;
    struct {
        OX_GenCommand g;
        int d0;
        int d1;
    } dd;
    struct {
        OX_GenCommand g;
        int s0;
        int s1;
        int s2;
        int s3;
    } ssss;
    struct {
        OX_GenCommand g;
        int c0;
    } c;
    struct {
        OX_GenCommand g;
        int p0;
        int s1;
    } ps;
} OX_Command;