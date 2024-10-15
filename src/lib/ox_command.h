static void
cmd_model_sd (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int s0, int d1)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->sd.s0 = s0;
    cmd->sd.d1 = d1;
}
static void
cmd_model_s (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int s0)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->s.s0 = s0;
}
static void
cmd_model_cs (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int c0, int s1)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->cs.c0 = c0;
    cmd->cs.s1 = s1;
}
static void
cmd_model_d (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int d0)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->d.d0 = d0;
}
static void
cmd_model_ss (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int s0, int s1)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->ss.s0 = s0;
    cmd->ss.s1 = s1;
}
static void
cmd_model_ssd (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int s0, int s1, int d2)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->ssd.s0 = s0;
    cmd->ssd.s1 = s1;
    cmd->ssd.d2 = d2;
}
static void
cmd_model_od (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int o0, int d1)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->od.o0 = o0;
    cmd->od.d1 = d1;
}
static void
cmd_model_cd (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int c0, int d1)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->cd.c0 = c0;
    cmd->cd.d1 = d1;
}
static void
cmd_model_pd (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int p0, int d1)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->pd.p0 = p0;
    cmd->pd.d1 = d1;
}
static void
cmd_model_td (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int t0, int d1)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->td.t0 = t0;
    cmd->td.d1 = d1;
}
static void
cmd_model_Td (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int T0, int d1)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->Td.T0 = T0;
    cmd->Td.d1 = d1;
}
static void
cmd_model_id (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int i0, int d1)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->id.i0 = i0;
    cmd->id.d1 = d1;
}
static void
cmd_model_is (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int i0, int s1)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->is.i0 = i0;
    cmd->is.s1 = s1;
}
static void
cmd_model_oid (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int o0, int i1, int d2)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->oid.o0 = o0;
    cmd->oid.i1 = i1;
    cmd->oid.d2 = d2;
}
static void
cmd_model_ois (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int o0, int i1, int s2)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->ois.o0 = o0;
    cmd->ois.i1 = i1;
    cmd->ois.s2 = s2;
}
static void
cmd_model_sss (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int s0, int s1, int s2)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->sss.s0 = s0;
    cmd->sss.s1 = s1;
    cmd->sss.s2 = s2;
}
static void
cmd_model_l (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int l0)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->l.l0 = l0;
}
static void
cmd_model_ol (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int o0, int l1)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->ol.o0 = o0;
    cmd->ol.l1 = l1;
}
static void
cmd_model_sl (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int s0, int l1)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->sl.s0 = s0;
    cmd->sl.l1 = l1;
}
static void
cmd_model_ll (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int l0, int l1)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->ll.l0 = l0;
    cmd->ll.l1 = l1;
}
static void
cmd_model_noarg (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty)
{
    add_cmd(ctxt, c, ty);
}
static void
cmd_model_dl (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int d0, int l1)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->dl.d0 = d0;
    cmd->dl.l1 = l1;
}
static void
cmd_model_dd (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int d0, int d1)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->dd.d0 = d0;
    cmd->dd.d1 = d1;
}
static void
cmd_model_ssss (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int s0, int s1, int s2, int s3)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->ssss.s0 = s0;
    cmd->ssss.s1 = s1;
    cmd->ssss.s2 = s2;
    cmd->ssss.s3 = s3;
}
static void
cmd_model_c (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int c0)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->c.c0 = c0;
}
static void
cmd_model_ps (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode ty, int p0, int s1)
{
    int cid = add_cmd(ctxt, c, ty);
    OX_Command *cmd = &ox_vector_item(&c->cmds, cid);

    cmd->ps.p0 = p0;
    cmd->ps.s1 = s1;
}

#define cmd_dup(ctxt, c, s0, d1)\
    OX_STMT_BEGIN\
        cmd_model_sd(ctxt, c, OX_BC_dup, s0, d1);\
    OX_STMT_END
#define cmd_not(ctxt, c, s0, d1)\
    OX_STMT_BEGIN\
        cmd_model_sd(ctxt, c, OX_BC_not, s0, d1);\
    OX_STMT_END
#define cmd_to_num(ctxt, c, s0, d1)\
    OX_STMT_BEGIN\
        cmd_model_sd(ctxt, c, OX_BC_to_num, s0, d1);\
    OX_STMT_END
#define cmd_rev(ctxt, c, s0, d1)\
    OX_STMT_BEGIN\
        cmd_model_sd(ctxt, c, OX_BC_rev, s0, d1);\
    OX_STMT_END
#define cmd_neg(ctxt, c, s0, d1)\
    OX_STMT_BEGIN\
        cmd_model_sd(ctxt, c, OX_BC_neg, s0, d1);\
    OX_STMT_END
#define cmd_typeof(ctxt, c, s0, d1)\
    OX_STMT_BEGIN\
        cmd_model_sd(ctxt, c, OX_BC_typeof, s0, d1);\
    OX_STMT_END
#define cmd_not_null(ctxt, c, s0, d1)\
    OX_STMT_BEGIN\
        cmd_model_sd(ctxt, c, OX_BC_not_null, s0, d1);\
    OX_STMT_END
#define cmd_global(ctxt, c, s0)\
    OX_STMT_BEGIN\
        cmd_model_s(ctxt, c, OX_BC_global, s0);\
    OX_STMT_END
#define cmd_owned(ctxt, c, c0, s1)\
    OX_STMT_BEGIN\
        cmd_model_cs(ctxt, c, OX_BC_owned, c0, s1);\
    OX_STMT_END
#define cmd_curr(ctxt, c, d0)\
    OX_STMT_BEGIN\
        cmd_model_d(ctxt, c, OX_BC_curr, d0);\
    OX_STMT_END
#define cmd_get_ptr(ctxt, c, s0, d1)\
    OX_STMT_BEGIN\
        cmd_model_sd(ctxt, c, OX_BC_get_ptr, s0, d1);\
    OX_STMT_END
#define cmd_get_value(ctxt, c, s0, d1)\
    OX_STMT_BEGIN\
        cmd_model_sd(ctxt, c, OX_BC_get_value, s0, d1);\
    OX_STMT_END
#define cmd_set_value(ctxt, c, s0, s1)\
    OX_STMT_BEGIN\
        cmd_model_ss(ctxt, c, OX_BC_set_value, s0, s1);\
    OX_STMT_END
#define cmd_add(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_add, s0, s1, d2);\
    OX_STMT_END
#define cmd_sub(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_sub, s0, s1, d2);\
    OX_STMT_END
#define cmd_match(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_match, s0, s1, d2);\
    OX_STMT_END
#define cmd_exp(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_exp, s0, s1, d2);\
    OX_STMT_END
#define cmd_mul(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_mul, s0, s1, d2);\
    OX_STMT_END
#define cmd_div(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_div, s0, s1, d2);\
    OX_STMT_END
#define cmd_mod(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_mod, s0, s1, d2);\
    OX_STMT_END
#define cmd_shl(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_shl, s0, s1, d2);\
    OX_STMT_END
#define cmd_shr(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_shr, s0, s1, d2);\
    OX_STMT_END
#define cmd_ushr(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_ushr, s0, s1, d2);\
    OX_STMT_END
#define cmd_lt(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_lt, s0, s1, d2);\
    OX_STMT_END
#define cmd_gt(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_gt, s0, s1, d2);\
    OX_STMT_END
#define cmd_le(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_le, s0, s1, d2);\
    OX_STMT_END
#define cmd_ge(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_ge, s0, s1, d2);\
    OX_STMT_END
#define cmd_instof(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_instof, s0, s1, d2);\
    OX_STMT_END
#define cmd_eq(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_eq, s0, s1, d2);\
    OX_STMT_END
#define cmd_ne(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_ne, s0, s1, d2);\
    OX_STMT_END
#define cmd_and(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_and, s0, s1, d2);\
    OX_STMT_END
#define cmd_xor(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_xor, s0, s1, d2);\
    OX_STMT_END
#define cmd_or(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_or, s0, s1, d2);\
    OX_STMT_END
#define cmd_load_null(ctxt, c, d0)\
    OX_STMT_BEGIN\
        cmd_model_d(ctxt, c, OX_BC_load_null, d0);\
    OX_STMT_END
#define cmd_load_true(ctxt, c, d0)\
    OX_STMT_BEGIN\
        cmd_model_d(ctxt, c, OX_BC_load_true, d0);\
    OX_STMT_END
#define cmd_load_false(ctxt, c, d0)\
    OX_STMT_BEGIN\
        cmd_model_d(ctxt, c, OX_BC_load_false, d0);\
    OX_STMT_END
#define cmd_this(ctxt, c, d0)\
    OX_STMT_BEGIN\
        cmd_model_d(ctxt, c, OX_BC_this, d0);\
    OX_STMT_END
#define cmd_this_b(ctxt, c, o0, d1)\
    OX_STMT_BEGIN\
        cmd_model_od(ctxt, c, OX_BC_this_b, o0, d1);\
    OX_STMT_END
#define cmd_argv(ctxt, c, d0)\
    OX_STMT_BEGIN\
        cmd_model_d(ctxt, c, OX_BC_argv, d0);\
    OX_STMT_END
#define cmd_get_cv(ctxt, c, c0, d1)\
    OX_STMT_BEGIN\
        cmd_model_cd(ctxt, c, OX_BC_get_cv, c0, d1);\
    OX_STMT_END
#define cmd_get_pp(ctxt, c, p0, d1)\
    OX_STMT_BEGIN\
        cmd_model_pd(ctxt, c, OX_BC_get_pp, p0, d1);\
    OX_STMT_END
#define cmd_get_lt(ctxt, c, t0, d1)\
    OX_STMT_BEGIN\
        cmd_model_td(ctxt, c, OX_BC_get_lt, t0, d1);\
    OX_STMT_END
#define cmd_get_ltt(ctxt, c, T0, d1)\
    OX_STMT_BEGIN\
        cmd_model_Td(ctxt, c, OX_BC_get_ltt, T0, d1);\
    OX_STMT_END
#define cmd_get_t(ctxt, c, i0, d1)\
    OX_STMT_BEGIN\
        cmd_model_id(ctxt, c, OX_BC_get_t, i0, d1);\
    OX_STMT_END
#define cmd_set_t(ctxt, c, i0, s1)\
    OX_STMT_BEGIN\
        cmd_model_is(ctxt, c, OX_BC_set_t, i0, s1);\
    OX_STMT_END
#define cmd_set_t_ac(ctxt, c, i0, s1)\
    OX_STMT_BEGIN\
        cmd_model_is(ctxt, c, OX_BC_set_t_ac, i0, s1);\
    OX_STMT_END
#define cmd_get_t_b(ctxt, c, o0, i1, d2)\
    OX_STMT_BEGIN\
        cmd_model_oid(ctxt, c, OX_BC_get_t_b, o0, i1, d2);\
    OX_STMT_END
#define cmd_set_t_b(ctxt, c, o0, i1, s2)\
    OX_STMT_BEGIN\
        cmd_model_ois(ctxt, c, OX_BC_set_t_b, o0, i1, s2);\
    OX_STMT_END
#define cmd_set_t_b_ac(ctxt, c, o0, i1, s2)\
    OX_STMT_BEGIN\
        cmd_model_ois(ctxt, c, OX_BC_set_t_b_ac, o0, i1, s2);\
    OX_STMT_END
#define cmd_get_n(ctxt, c, c0, d1)\
    OX_STMT_BEGIN\
        cmd_model_cd(ctxt, c, OX_BC_get_n, c0, d1);\
    OX_STMT_END
#define cmd_get_g(ctxt, c, c0, d1)\
    OX_STMT_BEGIN\
        cmd_model_cd(ctxt, c, OX_BC_get_g, c0, d1);\
    OX_STMT_END
#define cmd_get_r(ctxt, c, i0, d1)\
    OX_STMT_BEGIN\
        cmd_model_id(ctxt, c, OX_BC_get_r, i0, d1);\
    OX_STMT_END
#define cmd_get_p(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_get_p, s0, s1, d2);\
    OX_STMT_END
#define cmd_lookup_p(ctxt, c, s0, s1, d2)\
    OX_STMT_BEGIN\
        cmd_model_ssd(ctxt, c, OX_BC_lookup_p, s0, s1, d2);\
    OX_STMT_END
#define cmd_set_p(ctxt, c, s0, s1, s2)\
    OX_STMT_BEGIN\
        cmd_model_sss(ctxt, c, OX_BC_set_p, s0, s1, s2);\
    OX_STMT_END
#define cmd_get_a(ctxt, c, i0, d1)\
    OX_STMT_BEGIN\
        cmd_model_id(ctxt, c, OX_BC_get_a, i0, d1);\
    OX_STMT_END
#define cmd_throw(ctxt, c, s0)\
    OX_STMT_BEGIN\
        cmd_model_s(ctxt, c, OX_BC_throw, s0);\
    OX_STMT_END
#define cmd_ret(ctxt, c, s0)\
    OX_STMT_BEGIN\
        cmd_model_s(ctxt, c, OX_BC_ret, s0);\
    OX_STMT_END
#define cmd_jmp(ctxt, c, l0)\
    OX_STMT_BEGIN\
        cmd_model_l(ctxt, c, OX_BC_jmp, l0);\
    OX_STMT_END
#define cmd_deep_jmp(ctxt, c, o0, l1)\
    OX_STMT_BEGIN\
        cmd_model_ol(ctxt, c, OX_BC_deep_jmp, o0, l1);\
    OX_STMT_END
#define cmd_jt(ctxt, c, s0, l1)\
    OX_STMT_BEGIN\
        cmd_model_sl(ctxt, c, OX_BC_jt, s0, l1);\
    OX_STMT_END
#define cmd_jf(ctxt, c, s0, l1)\
    OX_STMT_BEGIN\
        cmd_model_sl(ctxt, c, OX_BC_jf, s0, l1);\
    OX_STMT_END
#define cmd_jnn(ctxt, c, s0, l1)\
    OX_STMT_BEGIN\
        cmd_model_sl(ctxt, c, OX_BC_jnn, s0, l1);\
    OX_STMT_END
#define cmd_str_start(ctxt, c, s0)\
    OX_STMT_BEGIN\
        cmd_model_s(ctxt, c, OX_BC_str_start, s0);\
        c->stack_level ++;\
    OX_STMT_END
#define cmd_str_start_t(ctxt, c, s0, s1)\
    OX_STMT_BEGIN\
        cmd_model_ss(ctxt, c, OX_BC_str_start_t, s0, s1);\
        c->stack_level ++;\
    OX_STMT_END
#define cmd_str_item(ctxt, c, s0)\
    OX_STMT_BEGIN\
        cmd_model_s(ctxt, c, OX_BC_str_item, s0);\
    OX_STMT_END
#define cmd_str_item_f(ctxt, c, c0, s1)\
    OX_STMT_BEGIN\
        cmd_model_cs(ctxt, c, OX_BC_str_item_f, c0, s1);\
    OX_STMT_END
#define cmd_str_end(ctxt, c, d0)\
    OX_STMT_BEGIN\
        cmd_model_d(ctxt, c, OX_BC_str_end, d0);\
        c->stack_level --;\
    OX_STMT_END
#define cmd_call_start(ctxt, c, s0, s1)\
    OX_STMT_BEGIN\
        cmd_model_ss(ctxt, c, OX_BC_call_start, s0, s1);\
        c->stack_level ++;\
    OX_STMT_END
#define cmd_arg(ctxt, c, s0)\
    OX_STMT_BEGIN\
        cmd_model_s(ctxt, c, OX_BC_arg, s0);\
    OX_STMT_END
#define cmd_arg_spread(ctxt, c, s0)\
    OX_STMT_BEGIN\
        cmd_model_s(ctxt, c, OX_BC_arg_spread, s0);\
    OX_STMT_END
#define cmd_call_end(ctxt, c, d0)\
    OX_STMT_BEGIN\
        cmd_model_d(ctxt, c, OX_BC_call_end, d0);\
        c->stack_level --;\
    OX_STMT_END
#define cmd_call_end_tail(ctxt, c, d0)\
    OX_STMT_BEGIN\
        cmd_model_d(ctxt, c, OX_BC_call_end_tail, d0);\
        c->stack_level --;\
    OX_STMT_END
#define cmd_try_start(ctxt, c, l0, l1)\
    OX_STMT_BEGIN\
        cmd_model_ll(ctxt, c, OX_BC_try_start, l0, l1);\
        c->stack_level ++;\
    OX_STMT_END
#define cmd_try_end(ctxt, c)\
    OX_STMT_BEGIN\
        cmd_model_noarg(ctxt, c, OX_BC_try_end);\
    OX_STMT_END
#define cmd_catch(ctxt, c, d0)\
    OX_STMT_BEGIN\
        cmd_model_d(ctxt, c, OX_BC_catch, d0);\
    OX_STMT_END
#define cmd_catch_end(ctxt, c)\
    OX_STMT_BEGIN\
        cmd_model_noarg(ctxt, c, OX_BC_catch_end);\
    OX_STMT_END
#define cmd_finally(ctxt, c)\
    OX_STMT_BEGIN\
        cmd_model_noarg(ctxt, c, OX_BC_finally);\
        c->stack_level --;\
    OX_STMT_END
#define cmd_sched(ctxt, c)\
    OX_STMT_BEGIN\
        cmd_model_noarg(ctxt, c, OX_BC_sched);\
    OX_STMT_END
#define cmd_sched_start(ctxt, c)\
    OX_STMT_BEGIN\
        cmd_model_noarg(ctxt, c, OX_BC_sched_start);\
        c->stack_level ++;\
    OX_STMT_END
#define cmd_yield(ctxt, c, s0, d1)\
    OX_STMT_BEGIN\
        cmd_model_sd(ctxt, c, OX_BC_yield, s0, d1);\
    OX_STMT_END
#define cmd_s_pop(ctxt, c)\
    OX_STMT_BEGIN\
        cmd_model_noarg(ctxt, c, OX_BC_s_pop);\
        c->stack_level --;\
    OX_STMT_END
#define cmd_iter_start(ctxt, c, s0)\
    OX_STMT_BEGIN\
        cmd_model_s(ctxt, c, OX_BC_iter_start, s0);\
        c->stack_level ++;\
    OX_STMT_END
#define cmd_iter_step(ctxt, c, d0, l1)\
    OX_STMT_BEGIN\
        cmd_model_dl(ctxt, c, OX_BC_iter_step, d0, l1);\
    OX_STMT_END
#define cmd_apat_start(ctxt, c, s0)\
    OX_STMT_BEGIN\
        cmd_model_s(ctxt, c, OX_BC_apat_start, s0);\
        c->stack_level ++;\
    OX_STMT_END
#define cmd_apat_next(ctxt, c)\
    OX_STMT_BEGIN\
        cmd_model_noarg(ctxt, c, OX_BC_apat_next);\
    OX_STMT_END
#define cmd_apat_get(ctxt, c, d0)\
    OX_STMT_BEGIN\
        cmd_model_d(ctxt, c, OX_BC_apat_get, d0);\
    OX_STMT_END
#define cmd_apat_rest(ctxt, c, d0)\
    OX_STMT_BEGIN\
        cmd_model_d(ctxt, c, OX_BC_apat_rest, d0);\
    OX_STMT_END
#define cmd_opat_start(ctxt, c, s0)\
    OX_STMT_BEGIN\
        cmd_model_s(ctxt, c, OX_BC_opat_start, s0);\
        c->stack_level ++;\
    OX_STMT_END
#define cmd_opat_get(ctxt, c, s0, d1)\
    OX_STMT_BEGIN\
        cmd_model_sd(ctxt, c, OX_BC_opat_get, s0, d1);\
    OX_STMT_END
#define cmd_opat_rest(ctxt, c, d0)\
    OX_STMT_BEGIN\
        cmd_model_d(ctxt, c, OX_BC_opat_rest, d0);\
    OX_STMT_END
#define cmd_a_new(ctxt, c, d0)\
    OX_STMT_BEGIN\
        cmd_model_d(ctxt, c, OX_BC_a_new, d0);\
    OX_STMT_END
#define cmd_a_start(ctxt, c, s0)\
    OX_STMT_BEGIN\
        cmd_model_s(ctxt, c, OX_BC_a_start, s0);\
        c->stack_level ++;\
    OX_STMT_END
#define cmd_a_next(ctxt, c)\
    OX_STMT_BEGIN\
        cmd_model_noarg(ctxt, c, OX_BC_a_next);\
    OX_STMT_END
#define cmd_a_item(ctxt, c, s0)\
    OX_STMT_BEGIN\
        cmd_model_s(ctxt, c, OX_BC_a_item, s0);\
    OX_STMT_END
#define cmd_a_spread(ctxt, c, s0)\
    OX_STMT_BEGIN\
        cmd_model_s(ctxt, c, OX_BC_a_spread, s0);\
    OX_STMT_END
#define cmd_a_end(ctxt, c)\
    OX_STMT_BEGIN\
        cmd_model_noarg(ctxt, c, OX_BC_a_end);\
        c->stack_level --;\
    OX_STMT_END
#define cmd_o_new(ctxt, c, d0)\
    OX_STMT_BEGIN\
        cmd_model_d(ctxt, c, OX_BC_o_new, d0);\
    OX_STMT_END
#define cmd_o_start(ctxt, c, s0)\
    OX_STMT_BEGIN\
        cmd_model_s(ctxt, c, OX_BC_o_start, s0);\
        c->stack_level ++;\
    OX_STMT_END
#define cmd_o_prop(ctxt, c, s0, s1)\
    OX_STMT_BEGIN\
        cmd_model_ss(ctxt, c, OX_BC_o_prop, s0, s1);\
    OX_STMT_END
#define cmd_o_spread(ctxt, c, s0)\
    OX_STMT_BEGIN\
        cmd_model_s(ctxt, c, OX_BC_o_spread, s0);\
    OX_STMT_END
#define cmd_p_start(ctxt, c)\
    OX_STMT_BEGIN\
        cmd_model_noarg(ctxt, c, OX_BC_p_start);\
        c->stack_level ++;\
    OX_STMT_END
#define cmd_p_get(ctxt, c, d0)\
    OX_STMT_BEGIN\
        cmd_model_d(ctxt, c, OX_BC_p_get, d0);\
    OX_STMT_END
#define cmd_p_rest(ctxt, c, d0)\
    OX_STMT_BEGIN\
        cmd_model_d(ctxt, c, OX_BC_p_rest, d0);\
    OX_STMT_END
#define cmd_f_new(ctxt, c, i0, d1)\
    OX_STMT_BEGIN\
        cmd_model_id(ctxt, c, OX_BC_f_new, i0, d1);\
    OX_STMT_END
#define cmd_c_new(ctxt, c, d0, d1)\
    OX_STMT_BEGIN\
        cmd_model_dd(ctxt, c, OX_BC_c_new, d0, d1);\
    OX_STMT_END
#define cmd_c_parent(ctxt, c, s0, s1)\
    OX_STMT_BEGIN\
        cmd_model_ss(ctxt, c, OX_BC_c_parent, s0, s1);\
    OX_STMT_END
#define cmd_c_const(ctxt, c, s0, s1, s2)\
    OX_STMT_BEGIN\
        cmd_model_sss(ctxt, c, OX_BC_c_const, s0, s1, s2);\
    OX_STMT_END
#define cmd_c_var(ctxt, c, s0, s1, s2)\
    OX_STMT_BEGIN\
        cmd_model_sss(ctxt, c, OX_BC_c_var, s0, s1, s2);\
    OX_STMT_END
#define cmd_c_acce(ctxt, c, s0, s1, s2, s3)\
    OX_STMT_BEGIN\
        cmd_model_ssss(ctxt, c, OX_BC_c_acce, s0, s1, s2, s3);\
    OX_STMT_END
#define cmd_c_ro_acce(ctxt, c, s0, s1, s2)\
    OX_STMT_BEGIN\
        cmd_model_sss(ctxt, c, OX_BC_c_ro_acce, s0, s1, s2);\
    OX_STMT_END
#define cmd_e_start(ctxt, c, s0)\
    OX_STMT_BEGIN\
        cmd_model_s(ctxt, c, OX_BC_e_start, s0);\
        c->stack_level ++;\
    OX_STMT_END
#define cmd_e_start_n(ctxt, c, c0, s1)\
    OX_STMT_BEGIN\
        cmd_model_cs(ctxt, c, OX_BC_e_start_n, c0, s1);\
        c->stack_level ++;\
    OX_STMT_END
#define cmd_e_item(ctxt, c, c0)\
    OX_STMT_BEGIN\
        cmd_model_c(ctxt, c, OX_BC_e_item, c0);\
    OX_STMT_END
#define cmd_b_start(ctxt, c, s0)\
    OX_STMT_BEGIN\
        cmd_model_s(ctxt, c, OX_BC_b_start, s0);\
        c->stack_level ++;\
    OX_STMT_END
#define cmd_b_start_n(ctxt, c, c0, s1)\
    OX_STMT_BEGIN\
        cmd_model_cs(ctxt, c, OX_BC_b_start_n, c0, s1);\
        c->stack_level ++;\
    OX_STMT_END
#define cmd_b_item(ctxt, c, c0)\
    OX_STMT_BEGIN\
        cmd_model_c(ctxt, c, OX_BC_b_item, c0);\
    OX_STMT_END
#define cmd_set_name(ctxt, c, s0, s1)\
    OX_STMT_BEGIN\
        cmd_model_ss(ctxt, c, OX_BC_set_name, s0, s1);\
    OX_STMT_END
#define cmd_set_name_g(ctxt, c, s0, s1)\
    OX_STMT_BEGIN\
        cmd_model_ss(ctxt, c, OX_BC_set_name_g, s0, s1);\
    OX_STMT_END
#define cmd_set_name_s(ctxt, c, s0, s1)\
    OX_STMT_BEGIN\
        cmd_model_ss(ctxt, c, OX_BC_set_name_s, s0, s1);\
    OX_STMT_END
#define cmd_set_scope(ctxt, c, s0, s1)\
    OX_STMT_BEGIN\
        cmd_model_ss(ctxt, c, OX_BC_set_scope, s0, s1);\
    OX_STMT_END
#define cmd_name_nn(ctxt, c, c0, s1)\
    OX_STMT_BEGIN\
        cmd_model_cs(ctxt, c, OX_BC_name_nn, c0, s1);\
    OX_STMT_END
#define cmd_prop_nn(ctxt, c, c0, s1)\
    OX_STMT_BEGIN\
        cmd_model_cs(ctxt, c, OX_BC_prop_nn, c0, s1);\
    OX_STMT_END
#define cmd_pprop_nn(ctxt, c, p0, s1)\
    OX_STMT_BEGIN\
        cmd_model_ps(ctxt, c, OX_BC_pprop_nn, p0, s1);\
    OX_STMT_END
#define cmd_stub(ctxt, c, l0)\
    OX_STMT_BEGIN\
        cmd_model_l(ctxt, c, OX_BC_stub, l0);\
        assert(c->stack_level == ox_vector_item(&c->labels, l0).stack_level);\
    OX_STMT_END
#define cmd_nop(ctxt, c)\
    OX_STMT_BEGIN\
        cmd_model_noarg(ctxt, c, OX_BC_nop);\
    OX_STMT_END

/*Bytecode name table.*/
static const char*
bytecode_names[] = {
    "dup",
    "not",
    "to_num",
    "rev",
    "neg",
    "typeof",
    "not_null",
    "global",
    "owned",
    "curr",
    "get_ptr",
    "get_value",
    "set_value",
    "add",
    "sub",
    "match",
    "exp",
    "mul",
    "div",
    "mod",
    "shl",
    "shr",
    "ushr",
    "lt",
    "gt",
    "le",
    "ge",
    "instof",
    "eq",
    "ne",
    "and",
    "xor",
    "or",
    "load_null",
    "load_true",
    "load_false",
    "this",
    "this_b",
    "argv",
    "get_cv",
    "get_pp",
    "get_lt",
    "get_ltt",
    "get_t",
    "set_t",
    "set_t_ac",
    "get_t_b",
    "set_t_b",
    "set_t_b_ac",
    "get_n",
    "get_g",
    "get_r",
    "get_p",
    "lookup_p",
    "set_p",
    "get_a",
    "throw",
    "ret",
    "jmp",
    "deep_jmp",
    "jt",
    "jf",
    "jnn",
    "str_start",
    "str_start_t",
    "str_item",
    "str_item_f",
    "str_end",
    "call_start",
    "arg",
    "arg_spread",
    "call_end",
    "call_end_tail",
    "try_start",
    "try_end",
    "catch",
    "catch_end",
    "finally",
    "sched",
    "sched_start",
    "yield",
    "s_pop",
    "iter_start",
    "iter_step",
    "apat_start",
    "apat_next",
    "apat_get",
    "apat_rest",
    "opat_start",
    "opat_get",
    "opat_rest",
    "a_new",
    "a_start",
    "a_next",
    "a_item",
    "a_spread",
    "a_end",
    "o_new",
    "o_start",
    "o_prop",
    "o_spread",
    "p_start",
    "p_get",
    "p_rest",
    "f_new",
    "c_new",
    "c_parent",
    "c_const",
    "c_var",
    "c_acce",
    "c_ro_acce",
    "e_start",
    "e_start_n",
    "e_item",
    "b_start",
    "b_start_n",
    "b_item",
    "set_name",
    "set_name_g",
    "set_name_s",
    "set_scope",
    "name_nn",
    "prop_nn",
    "pprop_nn",
    "stub",
    "nop"
};

/*Bytecode -> model map table.*/
static const OX_BcModel
bytecode_models[] = {
    OX_BC_MODEL_sd,
    OX_BC_MODEL_sd,
    OX_BC_MODEL_sd,
    OX_BC_MODEL_sd,
    OX_BC_MODEL_sd,
    OX_BC_MODEL_sd,
    OX_BC_MODEL_sd,
    OX_BC_MODEL_s,
    OX_BC_MODEL_cs,
    OX_BC_MODEL_d,
    OX_BC_MODEL_sd,
    OX_BC_MODEL_sd,
    OX_BC_MODEL_ss,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_d,
    OX_BC_MODEL_d,
    OX_BC_MODEL_d,
    OX_BC_MODEL_d,
    OX_BC_MODEL_od,
    OX_BC_MODEL_d,
    OX_BC_MODEL_cd,
    OX_BC_MODEL_pd,
    OX_BC_MODEL_td,
    OX_BC_MODEL_Td,
    OX_BC_MODEL_id,
    OX_BC_MODEL_is,
    OX_BC_MODEL_is,
    OX_BC_MODEL_oid,
    OX_BC_MODEL_ois,
    OX_BC_MODEL_ois,
    OX_BC_MODEL_cd,
    OX_BC_MODEL_cd,
    OX_BC_MODEL_id,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_ssd,
    OX_BC_MODEL_sss,
    OX_BC_MODEL_id,
    OX_BC_MODEL_s,
    OX_BC_MODEL_s,
    OX_BC_MODEL_l,
    OX_BC_MODEL_ol,
    OX_BC_MODEL_sl,
    OX_BC_MODEL_sl,
    OX_BC_MODEL_sl,
    OX_BC_MODEL_s,
    OX_BC_MODEL_ss,
    OX_BC_MODEL_s,
    OX_BC_MODEL_cs,
    OX_BC_MODEL_d,
    OX_BC_MODEL_ss,
    OX_BC_MODEL_s,
    OX_BC_MODEL_s,
    OX_BC_MODEL_d,
    OX_BC_MODEL_d,
    OX_BC_MODEL_ll,
    OX_BC_MODEL_noarg,
    OX_BC_MODEL_d,
    OX_BC_MODEL_noarg,
    OX_BC_MODEL_noarg,
    OX_BC_MODEL_noarg,
    OX_BC_MODEL_noarg,
    OX_BC_MODEL_sd,
    OX_BC_MODEL_noarg,
    OX_BC_MODEL_s,
    OX_BC_MODEL_dl,
    OX_BC_MODEL_s,
    OX_BC_MODEL_noarg,
    OX_BC_MODEL_d,
    OX_BC_MODEL_d,
    OX_BC_MODEL_s,
    OX_BC_MODEL_sd,
    OX_BC_MODEL_d,
    OX_BC_MODEL_d,
    OX_BC_MODEL_s,
    OX_BC_MODEL_noarg,
    OX_BC_MODEL_s,
    OX_BC_MODEL_s,
    OX_BC_MODEL_noarg,
    OX_BC_MODEL_d,
    OX_BC_MODEL_s,
    OX_BC_MODEL_ss,
    OX_BC_MODEL_s,
    OX_BC_MODEL_noarg,
    OX_BC_MODEL_d,
    OX_BC_MODEL_d,
    OX_BC_MODEL_id,
    OX_BC_MODEL_dd,
    OX_BC_MODEL_ss,
    OX_BC_MODEL_sss,
    OX_BC_MODEL_sss,
    OX_BC_MODEL_ssss,
    OX_BC_MODEL_sss,
    OX_BC_MODEL_s,
    OX_BC_MODEL_cs,
    OX_BC_MODEL_c,
    OX_BC_MODEL_s,
    OX_BC_MODEL_cs,
    OX_BC_MODEL_c,
    OX_BC_MODEL_ss,
    OX_BC_MODEL_ss,
    OX_BC_MODEL_ss,
    OX_BC_MODEL_ss,
    OX_BC_MODEL_cs,
    OX_BC_MODEL_cs,
    OX_BC_MODEL_ps,
    OX_BC_MODEL_l,
    OX_BC_MODEL_noarg
};

/*Bytecode model length table.*/
static const uint8_t
bytecode_len_table[] = {
    3,
    2,
    4,
    2,
    3,
    4,
    3,
    4,
    4,
    4,
    4,
    4,
    4,
    5,
    5,
    4,
    3,
    4,
    4,
    5,
    1,
    4,
    3,
    5,
    3,
    4
};

/*Mark the registers lifetime used by the command.*/
static void
bytecode_mark_regs (OX_Context *ctxt, OX_Compiler *c, int off, OX_Command *cmd)
{
    OX_BcModel m = bytecode_models[cmd->bc];
    OX_CompRegister *reg;

    switch (m) {
    case OX_BC_MODEL_sd:
        reg = &ox_vector_item(&c->regs, cmd->sd.s0);
        reg->off = off;
        reg = &ox_vector_item(&c->regs, cmd->sd.d1);
        reg->off = off;
        break;
    case OX_BC_MODEL_s:
        reg = &ox_vector_item(&c->regs, cmd->s.s0);
        reg->off = off;
        break;
    case OX_BC_MODEL_cs:
        reg = &ox_vector_item(&c->regs, cmd->cs.s1);
        reg->off = off;
        break;
    case OX_BC_MODEL_d:
        reg = &ox_vector_item(&c->regs, cmd->d.d0);
        reg->off = off;
        break;
    case OX_BC_MODEL_ss:
        reg = &ox_vector_item(&c->regs, cmd->ss.s0);
        reg->off = off;
        reg = &ox_vector_item(&c->regs, cmd->ss.s1);
        reg->off = off;
        break;
    case OX_BC_MODEL_ssd:
        reg = &ox_vector_item(&c->regs, cmd->ssd.s0);
        reg->off = off;
        reg = &ox_vector_item(&c->regs, cmd->ssd.s1);
        reg->off = off;
        reg = &ox_vector_item(&c->regs, cmd->ssd.d2);
        reg->off = off;
        break;
    case OX_BC_MODEL_od:
        reg = &ox_vector_item(&c->regs, cmd->od.d1);
        reg->off = off;
        break;
    case OX_BC_MODEL_cd:
        reg = &ox_vector_item(&c->regs, cmd->cd.d1);
        reg->off = off;
        break;
    case OX_BC_MODEL_pd:
        reg = &ox_vector_item(&c->regs, cmd->pd.d1);
        reg->off = off;
        break;
    case OX_BC_MODEL_td:
        reg = &ox_vector_item(&c->regs, cmd->td.d1);
        reg->off = off;
        break;
    case OX_BC_MODEL_Td:
        reg = &ox_vector_item(&c->regs, cmd->Td.d1);
        reg->off = off;
        break;
    case OX_BC_MODEL_id:
        reg = &ox_vector_item(&c->regs, cmd->id.d1);
        reg->off = off;
        break;
    case OX_BC_MODEL_is:
        reg = &ox_vector_item(&c->regs, cmd->is.s1);
        reg->off = off;
        break;
    case OX_BC_MODEL_oid:
        reg = &ox_vector_item(&c->regs, cmd->oid.d2);
        reg->off = off;
        break;
    case OX_BC_MODEL_ois:
        reg = &ox_vector_item(&c->regs, cmd->ois.s2);
        reg->off = off;
        break;
    case OX_BC_MODEL_sss:
        reg = &ox_vector_item(&c->regs, cmd->sss.s0);
        reg->off = off;
        reg = &ox_vector_item(&c->regs, cmd->sss.s1);
        reg->off = off;
        reg = &ox_vector_item(&c->regs, cmd->sss.s2);
        reg->off = off;
        break;
    case OX_BC_MODEL_l:
        break;
    case OX_BC_MODEL_ol:
        break;
    case OX_BC_MODEL_sl:
        reg = &ox_vector_item(&c->regs, cmd->sl.s0);
        reg->off = off;
        break;
    case OX_BC_MODEL_ll:
        break;
    case OX_BC_MODEL_noarg:
        break;
    case OX_BC_MODEL_dl:
        reg = &ox_vector_item(&c->regs, cmd->dl.d0);
        reg->off = off;
        break;
    case OX_BC_MODEL_dd:
        reg = &ox_vector_item(&c->regs, cmd->dd.d0);
        reg->off = off;
        reg = &ox_vector_item(&c->regs, cmd->dd.d1);
        reg->off = off;
        break;
    case OX_BC_MODEL_ssss:
        reg = &ox_vector_item(&c->regs, cmd->ssss.s0);
        reg->off = off;
        reg = &ox_vector_item(&c->regs, cmd->ssss.s1);
        reg->off = off;
        reg = &ox_vector_item(&c->regs, cmd->ssss.s2);
        reg->off = off;
        reg = &ox_vector_item(&c->regs, cmd->ssss.s3);
        reg->off = off;
        break;
    case OX_BC_MODEL_c:
        break;
    case OX_BC_MODEL_ps:
        reg = &ox_vector_item(&c->regs, cmd->ps.s1);
        reg->off = off;
        break;

    }
}

/*Allocate registers used by the command.*/
static OX_Result
bytecode_alloc_regs (OX_Context *ctxt, OX_Compiler *c, int off, OX_Command *cmd)
{
    OX_BcModel m = bytecode_models[cmd->bc];
    OX_CompRegister *reg;
    OX_Result r = OX_OK;

    switch (m) {
    case OX_BC_MODEL_sd:
        reg = &ox_vector_item(&c->regs, cmd->sd.s0);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        reg = &ox_vector_item(&c->regs, cmd->sd.d1);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_s:
        reg = &ox_vector_item(&c->regs, cmd->s.s0);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_cs:
        reg = &ox_vector_item(&c->regs, cmd->cs.s1);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_d:
        reg = &ox_vector_item(&c->regs, cmd->d.d0);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_ss:
        reg = &ox_vector_item(&c->regs, cmd->ss.s0);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        reg = &ox_vector_item(&c->regs, cmd->ss.s1);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_ssd:
        reg = &ox_vector_item(&c->regs, cmd->ssd.s0);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        reg = &ox_vector_item(&c->regs, cmd->ssd.s1);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        reg = &ox_vector_item(&c->regs, cmd->ssd.d2);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_od:
        reg = &ox_vector_item(&c->regs, cmd->od.d1);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_cd:
        reg = &ox_vector_item(&c->regs, cmd->cd.d1);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_pd:
        reg = &ox_vector_item(&c->regs, cmd->pd.d1);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_td:
        reg = &ox_vector_item(&c->regs, cmd->td.d1);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_Td:
        reg = &ox_vector_item(&c->regs, cmd->Td.d1);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_id:
        reg = &ox_vector_item(&c->regs, cmd->id.d1);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_is:
        reg = &ox_vector_item(&c->regs, cmd->is.s1);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_oid:
        reg = &ox_vector_item(&c->regs, cmd->oid.d2);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_ois:
        reg = &ox_vector_item(&c->regs, cmd->ois.s2);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_sss:
        reg = &ox_vector_item(&c->regs, cmd->sss.s0);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        reg = &ox_vector_item(&c->regs, cmd->sss.s1);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        reg = &ox_vector_item(&c->regs, cmd->sss.s2);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_l:
        break;
    case OX_BC_MODEL_ol:
        break;
    case OX_BC_MODEL_sl:
        reg = &ox_vector_item(&c->regs, cmd->sl.s0);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_ll:
        break;
    case OX_BC_MODEL_noarg:
        break;
    case OX_BC_MODEL_dl:
        reg = &ox_vector_item(&c->regs, cmd->dl.d0);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_dd:
        reg = &ox_vector_item(&c->regs, cmd->dd.d0);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        reg = &ox_vector_item(&c->regs, cmd->dd.d1);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_ssss:
        reg = &ox_vector_item(&c->regs, cmd->ssss.s0);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        reg = &ox_vector_item(&c->regs, cmd->ssss.s1);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        reg = &ox_vector_item(&c->regs, cmd->ssss.s2);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        reg = &ox_vector_item(&c->regs, cmd->ssss.s3);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;
    case OX_BC_MODEL_c:
        break;
    case OX_BC_MODEL_ps:
        reg = &ox_vector_item(&c->regs, cmd->ps.s1);
        if (reg->id == -1) {
            if ((reg->id = alloc_reg(ctxt, c, off, reg->off)) == -1)
                return OX_ERR;
        }
        break;

    }

    return r;
}

/*Store the bytecode to buffer.*/
static void
bytecode_store (OX_Context *ctxt, OX_Compiler *c, OX_Command *cmd, uint8_t *bc)
{
    OX_BcModel m = bytecode_models[cmd->bc];
    int id;
    uint8_t u8;
    uint16_t u16;

    switch (m) {
    case OX_BC_MODEL_sd:
        *bc ++ = cmd->bc;
        id = cmd->sd.s0;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        id = cmd->sd.d1;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_s:
        *bc ++ = cmd->bc;
        id = cmd->s.s0;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_cs:
        *bc ++ = cmd->bc;
        id = cmd->cs.c0;
        u16 = id;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
        id = cmd->cs.s1;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_d:
        *bc ++ = cmd->bc;
        id = cmd->d.d0;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_ss:
        *bc ++ = cmd->bc;
        id = cmd->ss.s0;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        id = cmd->ss.s1;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_ssd:
        *bc ++ = cmd->bc;
        id = cmd->ssd.s0;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        id = cmd->ssd.s1;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        id = cmd->ssd.d2;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_od:
        *bc ++ = cmd->bc;
        id = cmd->od.o0;
        u8 = id;
       *bc ++ = u8;
        id = cmd->od.d1;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_cd:
        *bc ++ = cmd->bc;
        id = cmd->cd.c0;
        u16 = id;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
        id = cmd->cd.d1;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_pd:
        *bc ++ = cmd->bc;
        id = cmd->pd.p0;
        u16 = id;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
        id = cmd->pd.d1;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_td:
        *bc ++ = cmd->bc;
        id = cmd->td.t0;
        u16 = id;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
        id = cmd->td.d1;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_Td:
        *bc ++ = cmd->bc;
        id = cmd->Td.T0;
        u16 = id;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
        id = cmd->Td.d1;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_id:
        *bc ++ = cmd->bc;
        id = cmd->id.i0;
        u16 = id;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
        id = cmd->id.d1;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_is:
        *bc ++ = cmd->bc;
        id = cmd->is.i0;
        u16 = id;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
        id = cmd->is.s1;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_oid:
        *bc ++ = cmd->bc;
        id = cmd->oid.o0;
        u8 = id;
       *bc ++ = u8;
        id = cmd->oid.i1;
        u16 = id;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
        id = cmd->oid.d2;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_ois:
        *bc ++ = cmd->bc;
        id = cmd->ois.o0;
        u8 = id;
       *bc ++ = u8;
        id = cmd->ois.i1;
        u16 = id;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
        id = cmd->ois.s2;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_sss:
        *bc ++ = cmd->bc;
        id = cmd->sss.s0;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        id = cmd->sss.s1;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        id = cmd->sss.s2;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_l:
        *bc ++ = cmd->bc;
        id = cmd->l.l0;
        u16 = c->labels.items[id].off;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
        break;
    case OX_BC_MODEL_ol:
        *bc ++ = cmd->bc;
        id = cmd->ol.o0;
        u8 = id;
       *bc ++ = u8;
        id = cmd->ol.l1;
        u16 = c->labels.items[id].off;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
        break;
    case OX_BC_MODEL_sl:
        *bc ++ = cmd->bc;
        id = cmd->sl.s0;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        id = cmd->sl.l1;
        u16 = c->labels.items[id].off;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
        break;
    case OX_BC_MODEL_ll:
        *bc ++ = cmd->bc;
        id = cmd->ll.l0;
        u16 = c->labels.items[id].off;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
        id = cmd->ll.l1;
        u16 = c->labels.items[id].off;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
        break;
    case OX_BC_MODEL_noarg:
        *bc ++ = cmd->bc;
        break;
    case OX_BC_MODEL_dl:
        *bc ++ = cmd->bc;
        id = cmd->dl.d0;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        id = cmd->dl.l1;
        u16 = c->labels.items[id].off;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
        break;
    case OX_BC_MODEL_dd:
        *bc ++ = cmd->bc;
        id = cmd->dd.d0;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        id = cmd->dd.d1;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_ssss:
        *bc ++ = cmd->bc;
        id = cmd->ssss.s0;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        id = cmd->ssss.s1;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        id = cmd->ssss.s2;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        id = cmd->ssss.s3;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;
    case OX_BC_MODEL_c:
        *bc ++ = cmd->bc;
        id = cmd->c.c0;
        u16 = id;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
        break;
    case OX_BC_MODEL_ps:
        *bc ++ = cmd->bc;
        id = cmd->ps.p0;
        u16 = id;
        *bc ++ = u16 >> 8;
        *bc ++ = u16 & 0xff;
        id = cmd->ps.s1;
        u8 = c->regs.items[id].id;
        *bc ++ = u8;
        break;

    }
}

/*Decompile the bytecode.*/
static int
bytecode_decompile (OX_Context *ctxt, OX_BcScript *s, uint8_t *bc, FILE *fp)
{
    OX_ByteCode t = *bc ++;
    OX_BcModel m = bytecode_models[t];
    uint8_t u8;
    uint16_t u16;

    fprintf(fp, "%-10s ", bytecode_names[t]);

    switch (m) {
    case OX_BC_MODEL_sd:
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_s:
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_cs:
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, "c%d(", u16);
        dump_const(ctxt, s, u16, fp);
        fprintf(fp, ") ");
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_d:
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_ss:
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_ssd:
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_od:
        u8 = *bc ++;
        fprintf(fp, "%d ", u8);
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_cd:
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, "c%d(", u16);
        dump_const(ctxt, s, u16, fp);
        fprintf(fp, ") ");
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_pd:
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, "p%d(", u16);
        dump_private(ctxt, s, u16, fp);
        fprintf(fp, ") ");
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_td:
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, "t%d(", u16);
        dump_local_text(ctxt, s, u16, fp);
        fprintf(fp, ") ");
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_Td:
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, "t%d(", u16);
        dump_local_templ(ctxt, s, u16, fp);
        fprintf(fp, ") ");
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_id:
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, "%d ", u16);
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_is:
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, "%d ", u16);
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_oid:
        u8 = *bc ++;
        fprintf(fp, "%d ", u8);
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, "%d ", u16);
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_ois:
        u8 = *bc ++;
        fprintf(fp, "%d ", u8);
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, "%d ", u16);
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_sss:
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_l:
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, "%d ", u16);
        break;
    case OX_BC_MODEL_ol:
        u8 = *bc ++;
        fprintf(fp, "%d ", u8);
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, "%d ", u16);
        break;
    case OX_BC_MODEL_sl:
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, "%d ", u16);
        break;
    case OX_BC_MODEL_ll:
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, "%d ", u16);
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, "%d ", u16);
        break;
    case OX_BC_MODEL_noarg:
        break;
    case OX_BC_MODEL_dl:
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, "%d ", u16);
        break;
    case OX_BC_MODEL_dd:
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_ssss:
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;
    case OX_BC_MODEL_c:
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, "c%d(", u16);
        dump_const(ctxt, s, u16, fp);
        fprintf(fp, ") ");
        break;
    case OX_BC_MODEL_ps:
        u16 = (bc[0] << 8) | bc[1];
        bc += 2;
        fprintf(fp, "p%d(", u16);
        dump_private(ctxt, s, u16, fp);
        fprintf(fp, ") ");
        u8 = *bc ++;
        fprintf(fp, "r%d ", u8);
        break;

    }

    return bytecode_len_table[m];
}
