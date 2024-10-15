    case OX_BC_dup: {
        uint8_t s0;
        uint8_t d1;
        s0 = bc[1];
        d1 = bc[2];
        r = do_dup(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_not: {
        uint8_t s0;
        uint8_t d1;
        s0 = bc[1];
        d1 = bc[2];
        r = do_not(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_to_num: {
        uint8_t s0;
        uint8_t d1;
        s0 = bc[1];
        d1 = bc[2];
        r = do_to_num(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_rev: {
        uint8_t s0;
        uint8_t d1;
        s0 = bc[1];
        d1 = bc[2];
        r = do_rev(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_neg: {
        uint8_t s0;
        uint8_t d1;
        s0 = bc[1];
        d1 = bc[2];
        r = do_neg(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_typeof: {
        uint8_t s0;
        uint8_t d1;
        s0 = bc[1];
        d1 = bc[2];
        r = do_typeof(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_not_null: {
        uint8_t s0;
        uint8_t d1;
        s0 = bc[1];
        d1 = bc[2];
        r = do_not_null(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_global: {
        uint8_t s0;
        s0 = bc[1];
        r = do_global(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_owned: {
        uint16_t c0;
        uint8_t s1;
        c0 = bc[1] << 8 | bc[2];
        s1 = bc[3];
        r = do_owned(ctxt, &rs, &rs.s->cvs[c0], ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_curr: {
        uint8_t d0;
        d0 = bc[1];
        r = do_curr(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_get_ptr: {
        uint8_t s0;
        uint8_t d1;
        s0 = bc[1];
        d1 = bc[2];
        r = do_get_ptr(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_get_value: {
        uint8_t s0;
        uint8_t d1;
        s0 = bc[1];
        d1 = bc[2];
        r = do_get_value(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_set_value: {
        uint8_t s0;
        uint8_t s1;
        s0 = bc[1];
        s1 = bc[2];
        r = do_set_value(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_add: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_add(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_sub: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_sub(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_match: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_match(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_exp: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_exp(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_mul: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_mul(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_div: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_div(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_mod: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_mod(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_shl: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_shl(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_shr: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_shr(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_ushr: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_ushr(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_lt: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_lt(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_gt: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_gt(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_le: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_le(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_ge: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_ge(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_instof: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_instof(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_eq: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_eq(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_ne: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_ne(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_and: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_and(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_xor: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_xor(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_or: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_or(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_load_null: {
        uint8_t d0;
        d0 = bc[1];
        r = do_load_null(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_load_true: {
        uint8_t d0;
        d0 = bc[1];
        r = do_load_true(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_load_false: {
        uint8_t d0;
        d0 = bc[1];
        r = do_load_false(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_this: {
        uint8_t d0;
        d0 = bc[1];
        r = do_this(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_this_b: {
        uint8_t o0;
        uint8_t d1;
        o0 = bc[1];
        d1 = bc[2];
        r = do_this_b(ctxt, &rs, o0, ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_argv: {
        uint8_t d0;
        d0 = bc[1];
        r = do_argv(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_get_cv: {
        uint16_t c0;
        uint8_t d1;
        c0 = bc[1] << 8 | bc[2];
        d1 = bc[3];
        r = do_get_cv(ctxt, &rs, &rs.s->cvs[c0], ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_get_pp: {
        uint16_t p0;
        uint8_t d1;
        p0 = bc[1] << 8 | bc[2];
        d1 = bc[3];
        r = do_get_pp(ctxt, &rs, &rs.s->pps[p0], ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_get_lt: {
        uint16_t t0;
        uint8_t d1;
        t0 = bc[1] << 8 | bc[2];
        d1 = bc[3];
        r = do_get_lt(ctxt, &rs, t0, ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_get_ltt: {
        uint16_t T0;
        uint8_t d1;
        T0 = bc[1] << 8 | bc[2];
        d1 = bc[3];
        r = do_get_ltt(ctxt, &rs, T0, ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_get_t: {
        uint16_t i0;
        uint8_t d1;
        i0 = bc[1] << 8 | bc[2];
        d1 = bc[3];
        r = do_get_t(ctxt, &rs, i0, ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_set_t: {
        uint16_t i0;
        uint8_t s1;
        i0 = bc[1] << 8 | bc[2];
        s1 = bc[3];
        r = do_set_t(ctxt, &rs, i0, ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_set_t_ac: {
        uint16_t i0;
        uint8_t s1;
        i0 = bc[1] << 8 | bc[2];
        s1 = bc[3];
        r = do_set_t_ac(ctxt, &rs, i0, ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_get_t_b: {
        uint8_t o0;
        uint16_t i1;
        uint8_t d2;
        o0 = bc[1];
        i1 = bc[2] << 8 | bc[3];
        d2 = bc[4];
        r = do_get_t_b(ctxt, &rs, o0, i1, ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 5;
        break;
    }
    case OX_BC_set_t_b: {
        uint8_t o0;
        uint16_t i1;
        uint8_t s2;
        o0 = bc[1];
        i1 = bc[2] << 8 | bc[3];
        s2 = bc[4];
        r = do_set_t_b(ctxt, &rs, o0, i1, ox_values_item(ctxt, rs.regs, s2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 5;
        break;
    }
    case OX_BC_set_t_b_ac: {
        uint8_t o0;
        uint16_t i1;
        uint8_t s2;
        o0 = bc[1];
        i1 = bc[2] << 8 | bc[3];
        s2 = bc[4];
        r = do_set_t_b_ac(ctxt, &rs, o0, i1, ox_values_item(ctxt, rs.regs, s2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 5;
        break;
    }
    case OX_BC_get_n: {
        uint16_t c0;
        uint8_t d1;
        c0 = bc[1] << 8 | bc[2];
        d1 = bc[3];
        r = do_get_n(ctxt, &rs, &rs.s->cvs[c0], ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_get_g: {
        uint16_t c0;
        uint8_t d1;
        c0 = bc[1] << 8 | bc[2];
        d1 = bc[3];
        r = do_get_g(ctxt, &rs, &rs.s->cvs[c0], ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_get_r: {
        uint16_t i0;
        uint8_t d1;
        i0 = bc[1] << 8 | bc[2];
        d1 = bc[3];
        r = do_get_r(ctxt, &rs, i0, ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_get_p: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_get_p(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_lookup_p: {
        uint8_t s0;
        uint8_t s1;
        uint8_t d2;
        s0 = bc[1];
        s1 = bc[2];
        d2 = bc[3];
        r = do_lookup_p(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, d2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_set_p: {
        uint8_t s0;
        uint8_t s1;
        uint8_t s2;
        s0 = bc[1];
        s1 = bc[2];
        s2 = bc[3];
        r = do_set_p(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, s2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_get_a: {
        uint16_t i0;
        uint8_t d1;
        i0 = bc[1] << 8 | bc[2];
        d1 = bc[3];
        r = do_get_a(ctxt, &rs, i0, ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_throw: {
        uint8_t s0;
        s0 = bc[1];
        r = do_throw(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_ret: {
        uint8_t s0;
        s0 = bc[1];
        r = do_ret(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_jmp: {
        uint16_t l0;
        l0 = bc[1] << 8 | bc[2];
        r = do_jmp(ctxt, &rs, l0);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_deep_jmp: {
        uint8_t o0;
        uint16_t l1;
        o0 = bc[1];
        l1 = bc[2] << 8 | bc[3];
        r = do_deep_jmp(ctxt, &rs, o0, l1);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_jt: {
        uint8_t s0;
        uint16_t l1;
        s0 = bc[1];
        l1 = bc[2] << 8 | bc[3];
        r = do_jt(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), l1);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_jf: {
        uint8_t s0;
        uint16_t l1;
        s0 = bc[1];
        l1 = bc[2] << 8 | bc[3];
        r = do_jf(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), l1);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_jnn: {
        uint8_t s0;
        uint16_t l1;
        s0 = bc[1];
        l1 = bc[2] << 8 | bc[3];
        r = do_jnn(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), l1);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_str_start: {
        uint8_t s0;
        s0 = bc[1];
        r = do_str_start(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_str_start_t: {
        uint8_t s0;
        uint8_t s1;
        s0 = bc[1];
        s1 = bc[2];
        r = do_str_start_t(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_str_item: {
        uint8_t s0;
        s0 = bc[1];
        r = do_str_item(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_str_item_f: {
        uint16_t c0;
        uint8_t s1;
        c0 = bc[1] << 8 | bc[2];
        s1 = bc[3];
        r = do_str_item_f(ctxt, &rs, &rs.s->cvs[c0], ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_str_end: {
        uint8_t d0;
        d0 = bc[1];
        r = do_str_end(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_call_start: {
        uint8_t s0;
        uint8_t s1;
        s0 = bc[1];
        s1 = bc[2];
        r = do_call_start(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_arg: {
        uint8_t s0;
        s0 = bc[1];
        r = do_arg(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_arg_spread: {
        uint8_t s0;
        s0 = bc[1];
        r = do_arg_spread(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_call_end: {
        uint8_t d0;
        d0 = bc[1];
        r = do_call_end(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_call_end_tail: {
        uint8_t d0;
        d0 = bc[1];
        r = do_call_end_tail(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_try_start: {
        uint16_t l0;
        uint16_t l1;
        l0 = bc[1] << 8 | bc[2];
        l1 = bc[3] << 8 | bc[4];
        r = do_try_start(ctxt, &rs, l0, l1);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 5;
        break;
    }
    case OX_BC_try_end: {
        r = do_try_end(ctxt, &rs);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 1;
        break;
    }
    case OX_BC_catch: {
        uint8_t d0;
        d0 = bc[1];
        r = do_catch(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_catch_end: {
        r = do_catch_end(ctxt, &rs);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 1;
        break;
    }
    case OX_BC_finally: {
        r = do_finally(ctxt, &rs);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 1;
        break;
    }
    case OX_BC_sched: {
        r = do_sched(ctxt, &rs);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 1;
        break;
    }
    case OX_BC_sched_start: {
        r = do_sched_start(ctxt, &rs);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 1;
        break;
    }
    case OX_BC_yield: {
        uint8_t s0;
        uint8_t d1;
        s0 = bc[1];
        d1 = bc[2];
        r = do_yield(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_s_pop: {
        r = do_s_pop(ctxt, &rs);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 1;
        break;
    }
    case OX_BC_iter_start: {
        uint8_t s0;
        s0 = bc[1];
        r = do_iter_start(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_iter_step: {
        uint8_t d0;
        uint16_t l1;
        d0 = bc[1];
        l1 = bc[2] << 8 | bc[3];
        r = do_iter_step(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0), l1);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_apat_start: {
        uint8_t s0;
        s0 = bc[1];
        r = do_apat_start(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_apat_next: {
        r = do_apat_next(ctxt, &rs);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 1;
        break;
    }
    case OX_BC_apat_get: {
        uint8_t d0;
        d0 = bc[1];
        r = do_apat_get(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_apat_rest: {
        uint8_t d0;
        d0 = bc[1];
        r = do_apat_rest(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_opat_start: {
        uint8_t s0;
        s0 = bc[1];
        r = do_opat_start(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_opat_get: {
        uint8_t s0;
        uint8_t d1;
        s0 = bc[1];
        d1 = bc[2];
        r = do_opat_get(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_opat_rest: {
        uint8_t d0;
        d0 = bc[1];
        r = do_opat_rest(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_a_new: {
        uint8_t d0;
        d0 = bc[1];
        r = do_a_new(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_a_start: {
        uint8_t s0;
        s0 = bc[1];
        r = do_a_start(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_a_next: {
        r = do_a_next(ctxt, &rs);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 1;
        break;
    }
    case OX_BC_a_item: {
        uint8_t s0;
        s0 = bc[1];
        r = do_a_item(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_a_spread: {
        uint8_t s0;
        s0 = bc[1];
        r = do_a_spread(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_a_end: {
        r = do_a_end(ctxt, &rs);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 1;
        break;
    }
    case OX_BC_o_new: {
        uint8_t d0;
        d0 = bc[1];
        r = do_o_new(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_o_start: {
        uint8_t s0;
        s0 = bc[1];
        r = do_o_start(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_o_prop: {
        uint8_t s0;
        uint8_t s1;
        s0 = bc[1];
        s1 = bc[2];
        r = do_o_prop(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_o_spread: {
        uint8_t s0;
        s0 = bc[1];
        r = do_o_spread(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_p_start: {
        r = do_p_start(ctxt, &rs);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 1;
        break;
    }
    case OX_BC_p_get: {
        uint8_t d0;
        d0 = bc[1];
        r = do_p_get(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_p_rest: {
        uint8_t d0;
        d0 = bc[1];
        r = do_p_rest(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_f_new: {
        uint16_t i0;
        uint8_t d1;
        i0 = bc[1] << 8 | bc[2];
        d1 = bc[3];
        r = do_f_new(ctxt, &rs, i0, ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_c_new: {
        uint8_t d0;
        uint8_t d1;
        d0 = bc[1];
        d1 = bc[2];
        r = do_c_new(ctxt, &rs, ox_values_item(ctxt, rs.regs, d0), ox_values_item(ctxt, rs.regs, d1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_c_parent: {
        uint8_t s0;
        uint8_t s1;
        s0 = bc[1];
        s1 = bc[2];
        r = do_c_parent(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_c_const: {
        uint8_t s0;
        uint8_t s1;
        uint8_t s2;
        s0 = bc[1];
        s1 = bc[2];
        s2 = bc[3];
        r = do_c_const(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, s2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_c_var: {
        uint8_t s0;
        uint8_t s1;
        uint8_t s2;
        s0 = bc[1];
        s1 = bc[2];
        s2 = bc[3];
        r = do_c_var(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, s2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_c_acce: {
        uint8_t s0;
        uint8_t s1;
        uint8_t s2;
        uint8_t s3;
        s0 = bc[1];
        s1 = bc[2];
        s2 = bc[3];
        s3 = bc[4];
        r = do_c_acce(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, s2), ox_values_item(ctxt, rs.regs, s3));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 5;
        break;
    }
    case OX_BC_c_ro_acce: {
        uint8_t s0;
        uint8_t s1;
        uint8_t s2;
        s0 = bc[1];
        s1 = bc[2];
        s2 = bc[3];
        r = do_c_ro_acce(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1), ox_values_item(ctxt, rs.regs, s2));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_e_start: {
        uint8_t s0;
        s0 = bc[1];
        r = do_e_start(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_e_start_n: {
        uint16_t c0;
        uint8_t s1;
        c0 = bc[1] << 8 | bc[2];
        s1 = bc[3];
        r = do_e_start_n(ctxt, &rs, &rs.s->cvs[c0], ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_e_item: {
        uint16_t c0;
        c0 = bc[1] << 8 | bc[2];
        r = do_e_item(ctxt, &rs, &rs.s->cvs[c0]);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_b_start: {
        uint8_t s0;
        s0 = bc[1];
        r = do_b_start(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 2;
        break;
    }
    case OX_BC_b_start_n: {
        uint16_t c0;
        uint8_t s1;
        c0 = bc[1] << 8 | bc[2];
        s1 = bc[3];
        r = do_b_start_n(ctxt, &rs, &rs.s->cvs[c0], ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_b_item: {
        uint16_t c0;
        c0 = bc[1] << 8 | bc[2];
        r = do_b_item(ctxt, &rs, &rs.s->cvs[c0]);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_set_name: {
        uint8_t s0;
        uint8_t s1;
        s0 = bc[1];
        s1 = bc[2];
        r = do_set_name(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_set_name_g: {
        uint8_t s0;
        uint8_t s1;
        s0 = bc[1];
        s1 = bc[2];
        r = do_set_name_g(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_set_name_s: {
        uint8_t s0;
        uint8_t s1;
        s0 = bc[1];
        s1 = bc[2];
        r = do_set_name_s(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_set_scope: {
        uint8_t s0;
        uint8_t s1;
        s0 = bc[1];
        s1 = bc[2];
        r = do_set_scope(ctxt, &rs, ox_values_item(ctxt, rs.regs, s0), ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 3;
        break;
    }
    case OX_BC_name_nn: {
        uint16_t c0;
        uint8_t s1;
        c0 = bc[1] << 8 | bc[2];
        s1 = bc[3];
        r = do_name_nn(ctxt, &rs, &rs.s->cvs[c0], ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_prop_nn: {
        uint16_t c0;
        uint8_t s1;
        c0 = bc[1] << 8 | bc[2];
        s1 = bc[3];
        r = do_prop_nn(ctxt, &rs, &rs.s->cvs[c0], ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_pprop_nn: {
        uint16_t p0;
        uint8_t s1;
        p0 = bc[1] << 8 | bc[2];
        s1 = bc[3];
        r = do_pprop_nn(ctxt, &rs, &rs.s->pps[p0], ox_values_item(ctxt, rs.regs, s1));
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 4;
        break;
    }
    case OX_BC_nop: {
        r = do_nop(ctxt, &rs);
        if (r == OX_JUMP)
            break;
        if (r != OX_OK)
            goto pop;
        rs.frame->ip += 1;
        break;
    }

