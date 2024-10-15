/******************************************************************************
 *                                 OX Language                                *
 *                                                                            *
 * Copyright 2024 Gong Ke                                                     *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person obtaining a    *
 * copy of this software and associated documentation files (the              *
 * "Software"), to deal in the Software without restriction, including        *
 * without limitation the rights to use, copy, modify, merge, publish,        *
 * distribute, sublicense, and/or sell copies of the Software, and to permit  *
 * persons to whom the Software is furnished to do so, subject to the         *
 * following conditions:                                                      *
 *                                                                            *
 * The above copyright notice and this permission notice shall be included    *
 * in all copies or substantial portions of the Software.                     *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS    *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF                 *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN  *
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR      *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE  *
 * USE OR OTHER DEALINGS IN THE SOFTWARE.                                     *
 ******************************************************************************/

/**
 * @file
 * GICallableInfo.
 */

/** Call state.*/
typedef struct {
    OX_GICallable *c;        /**< Callable data.*/
    OX_GICtxt      gic;      /**< GI context.*/
    OX_GIOwn      *in_owns;  /**< Input arguments' own flags.*/
    OX_GIOwn      *out_owns; /**< Output arguments' own flags.*/
    GIArgument     ret_arg;  /**< Return argument.*/
    OX_GIOwn       ret_own;  /**< Return argument's own flag.*/
    OX_Value      *thiz;     /**< This arugent.*/
    OX_Value      *args;     /**< Arguments.*/
    size_t         argc;     /**< Arguments' count.*/
    GError        *error;    /**< GError.*/
} OX_GICallState;

/*Convert GITransfer to OXGIOwn*/
static OX_GIOwn
giown_from_transfer (GITransfer t)
{
    OX_GIOwn own = OX_GI_OWN_NOTHING;

    switch (t) {
    case GI_TRANSFER_NOTHING:
        own = OX_GI_OWN_NOTHING;
        break;
    case GI_TRANSFER_CONTAINER:
        own = OX_GI_OWN_CONTAINER;
        break;
    case GI_TRANSFER_EVERYTHING:
        own = OX_GI_OWN_EVERYTHING;
        break;
    }

    return own;
}

/*Scan reference objects in the GI callable.*/
static void
gicallable_scan (OX_Context *ctxt, OX_GICallable *c)
{
    ox_gc_scan_value(ctxt, &c->repo->v);
}

/*Initialize the GI callable.*/
static OX_Result
gicallable_init (OX_Context *ctxt, OX_GIRepository *repo, OX_GICallable *c, GICallableInfo *ci, OX_GICallDir dir)
{
    OX_GIArgType **pin, **pout;
    int i;

    c->repo = repo;
    c->dir = dir;
    c->can_throw = gi_callable_info_can_throw_gerror(ci);
    c->n_args = gi_callable_info_get_n_args(ci);

    if (c->n_args) {
        if (!OX_NEW_N_0(ctxt, c->atypes, c->n_args))
            return ox_throw_no_mem_error(ctxt);
    }

    if (gi_callable_info_is_method(ci)) {
        c->inst_bi = gi_base_info_get_container(GI_BASE_INFO(ci));
        c->inst_transfer = giown_from_transfer(gi_callable_info_get_instance_ownership_transfer(ci));

        c->n_in_args ++;
    }

    pin = &c->in_atypes;
    pout = &c->out_atypes;

    for (i = 0; i < c->n_args; i ++) {
        OX_GIArgType *atype = &c->atypes[i];
        GIArgInfo *ai;

        ai = gi_callable_info_get_arg(ci, i);

        atype->transfer = giown_from_transfer(gi_arg_info_get_ownership_transfer(ai));
        atype->ti = gi_arg_info_get_type_info(ai);
        atype->dir = gi_arg_info_get_direction(ai);

        if (!gi_arg_info_is_skip(ai)) {
            switch (gi_arg_info_get_direction(ai)) {
            case GI_DIRECTION_IN:
                atype->in_arg_idx = c->n_in_args ++;
                *pin = atype;
                pin = &atype->in_next;
                break;
            case GI_DIRECTION_OUT:
                atype->out_arg_idx = c->n_out_args ++;
                *pout = atype;
                pout = &atype->out_next;
                break;
            case GI_DIRECTION_INOUT:
                atype->in_arg_idx = c->n_in_args ++;
                *pin = atype;
                pin = &atype->in_next;
                atype->out_arg_idx = c->n_out_args ++;
                *pout = atype;
                pout = &atype->out_next;
                break;
            }
        }

        gi_base_info_unref(ai);
    }

    if (!gi_callable_info_skip_return(ci)) {
        c->ret_ti = gi_callable_info_get_return_type(ci);

        if (gi_type_info_get_tag(c->ret_ti) == GI_TYPE_TAG_VOID) {
            gi_base_info_unref(c->ret_ti);
            c->ret_ti = NULL;
        }
    }

    if (c->ret_ti) {
        c->ret_transfer = giown_from_transfer(gi_callable_info_get_caller_owns(ci));
    }

    return OX_OK;
}

/*Release the GI callable.*/
static void
gicallable_deinit (OX_Context *ctxt, OX_GICallable *c)
{
    if (c->atypes) {
        int i;

        for (i = 0; i < c->n_args; i ++) {
            OX_GIArgType *atype = &c->atypes[i];

            gi_base_info_unref(atype->ti);
        }
        OX_DEL_N(ctxt, c->atypes, c->n_args);
    }

    if (c->ret_ti)
        gi_base_info_unref(c->ret_ti);
}

/*Initialize the call state.*/
static void
gicallstate_init (OX_GICallState *cs, OX_GICallable *c, GIArgument *in_args, GIArgument *out_args, OX_GIOwn *in_owns, OX_GIOwn *out_owns)
{
    cs->c = c;
    cs->in_owns = in_owns;
    cs->out_owns = out_owns;
    cs->error = NULL;

    gictxt_init(&cs->gic, c->repo, c->atypes, in_args, out_args);

    if (c->n_in_args)
        memset(in_owns, 0, sizeof(OX_GIOwn) * c->n_in_args);
    if (c->n_out_args)
        memset(out_owns, 0, sizeof(OX_GIOwn) * c->n_out_args);

    cs->ret_own = OX_GI_OWN_NOTHING;
}

/*Release the call state.*/
static void
gicallstate_deinit (OX_Context *ctxt, OX_GICallState *cs)
{
    OX_GIArgType *atype;
    GIArgument *arg = cs->gic.in_args;
    OX_GIOwn *own = cs->in_owns;

    if (cs->c->inst_bi) {
        arg ++;
        own ++;
    }

    for (atype = cs->c->in_atypes; atype; atype = atype->in_next) {
        giargument_free(ctxt, &cs->gic, arg, *own, atype->ti);
        arg ++;
        own ++;
    }

    arg = cs->gic.out_args;
    own = cs->out_owns;
    for (atype = cs->c->out_atypes; atype; atype = atype->out_next) {
        giargument_free(ctxt, &cs->gic, arg, *own, atype->ti);
        arg ++;
        own ++;
    }

    if (cs->c->ret_ti)
        giargument_free(ctxt, &cs->gic, &cs->ret_arg, cs->ret_own, cs->c->ret_ti);

    if (cs->error)
        g_error_free(cs->error);
}

/*Prepare input parameters in the call state for C function's OX wrapper.*/
static OX_Result
gicallstate_input_ox (OX_Context *ctxt, OX_GICallState *cs, OX_Value *thiz, OX_Value *args, size_t argc)
{
    GIArgument *arg = cs->gic.in_args;
    OX_GIOwn *own = cs->in_owns;
    OX_Value *argv;
    size_t aid = 0;
    OX_GIArgType *atype;
    OX_Result r;

    cs->thiz = thiz;
    cs->args = args;
    cs->argc = argc;

    if (cs->c->inst_bi) {
        if (GI_IS_ENUM_INFO(cs->c->inst_bi)) {
            int32_t i;

            argv = ox_argument(ctxt, args, argc, aid ++);
            if ((r = ox_to_int32(ctxt, argv, &i)) == OX_ERR)
                goto end;

            arg->v_int32 = i;
        } else if (GI_IS_STRUCT_INFO(cs->c->inst_bi) || GI_IS_UNION_INFO(cs->c->inst_bi)) {
            OX_GIInst *inst;
            
            if (!(inst = giinst_get(ctxt, thiz))) {
                r = OX_ERR;
                goto end;
            }

            if (!inst->he.key) {
                r = ox_throw_null_error(ctxt, OX_TEXT("the instance is already freed"));
                goto end;
            }

            if (!gi_base_info_equal(cs->c->inst_bi, GI_BASE_INFO(inst->type->rti))) {
                r = ox_throw_type_error(ctxt, OX_TEXT("the value is not in type \"%s\""),
                        gi_base_info_get_name(cs->c->inst_bi));
                goto end;
            }

            arg->v_pointer = inst->he.key;
        } else if (GI_IS_OBJECT_INFO(cs->c->inst_bi) || GI_IS_INTERFACE_INFO(cs->c->inst_bi) ) {
            OX_GIInst *inst;
            GType gty;
            
            if (!(inst = giinst_get(ctxt, thiz))) {
                r = OX_ERR;
                goto end;
            }

            if (!inst->he.key) {
                r = ox_throw_null_error(ctxt, OX_TEXT("the instance is already freed"));
                goto end;
            }

            gty = gi_registered_type_info_get_g_type(inst->type->rti);

            if (!G_TYPE_CHECK_INSTANCE_TYPE(inst->he.key, gty)) {
                r = ox_throw_type_error(ctxt, OX_TEXT("the value is not in type \"%s\""),
                        gi_base_info_get_name(cs->c->inst_bi));
                goto end;
            }

            arg->v_pointer = inst->he.key;
        }

        arg ++;
        own ++;
    }

    for (atype = cs->c->in_atypes; atype; atype = atype->in_next) {
        argv = ox_argument(ctxt, args, argc, aid);

        if ((r = giargument_from_value(ctxt, &cs->gic, arg, own, atype->ti, argv)) == OX_ERR)
            goto end;

        arg ++;
        own ++;
        aid ++;
    }

    r = OX_OK;
end:
    return r;
}

/*Convert the return value and output parameters for C function's OX wrapper.*/
static OX_Result
gicallstate_output_ox (OX_Context *ctxt, OX_GICallState *cs, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, item)
    OX_Value *v;
    OX_GIArgType *atype;
    GIArgument *arg;
    OX_GIOwn *own;
    size_t arg_id = 0;
    size_t ret_id = 0;
    OX_Result r;
    int ret_len;

    arg = cs->gic.in_args;
    own = cs->in_owns;

    /*Sovle the instance parameter.*/
    if (cs->c->inst_bi) {
        if (GI_IS_ENUM_INFO(cs->c->inst_bi))
            arg_id ++;

        if (cs->c->inst_transfer) {
            if (GI_IS_STRUCT_INFO(cs->c->inst_bi)
                    || GI_IS_UNION_INFO(cs->c->inst_bi)
                    || GI_IS_OBJECT_INFO(cs->c->inst_bi)
                    || GI_IS_INTERFACE_INFO(cs->c->inst_bi))
                giinst_set_null(ctxt, cs->thiz);
        }

        arg ++;
        own ++;
    }

    /*Solve the input arguments.*/
    for (atype = cs->c->in_atypes; atype; atype = atype->in_next) {
        OX_Value *argv = ox_argument(ctxt, cs->args, cs->argc, arg_id);

        if (atype->transfer) {
            if (GI_IS_STRUCT_INFO(atype->ti)
                    || GI_IS_UNION_INFO(atype->ti)
                    || GI_IS_OBJECT_INFO(atype->ti)
                    || GI_IS_INTERFACE_INFO(atype->ti)) {
                giinst_set_null(ctxt, argv);
            } else if (ox_value_is_cvalue(ctxt, argv)) {
                ox_cvalue_set_null(ctxt, argv);
            }
        }

        arg ++;
        own ++;
    }

    /*Convert return value and output parameters.*/
    ret_len = cs->c->n_out_args;
    if (cs->c->ret_ti)
        ret_len ++;

    if (ret_len) {
        if (ret_len > 1) {
            if ((r = ox_array_new(ctxt, rv, ret_len)) == OX_ERR)
                goto end;
        }

        if (cs->c->ret_ti) {
            v = (ret_len > 1) ? item : rv;

            cs->ret_own = cs->c->ret_transfer;

            if ((r = giargument_to_value(ctxt, &cs->gic, &cs->ret_arg, &cs->ret_own, cs->c->ret_ti, v)) == OX_ERR)
                goto end;

            if (ret_len > 1) {
                if ((r = ox_array_set_item(ctxt, rv, ret_id, item)) == OX_ERR)
                    goto end;
            }

            ret_id ++;
        }

        for (atype = cs->c->out_atypes; atype; atype = atype->out_next) {
            v = (ret_len > 1) ? item : rv;

            if (atype->dir != GI_DIRECTION_OUT) {
                arg = &cs->gic.in_args[atype->in_arg_idx];
                own = &cs->in_owns[atype->in_arg_idx];
            } else {
                arg = &cs->gic.out_args[atype->out_arg_idx];
                own = &cs->out_owns[atype->out_arg_idx];
            }

            *own = atype->transfer;

            if ((r = giargument_to_value(ctxt, &cs->gic, arg, own, atype->ti, v)) == OX_ERR)
                goto end;

            if (ret_len > 1) {
                if ((r = ox_array_set_item(ctxt, rv, ret_id, item)) == OX_ERR)
                    goto end;
            }

            ret_id ++;
        }
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, item)
    return r;
}

/*Convert input parameters form C to OX.*/
static OX_Result
gicallstate_input_c (OX_Context *ctxt, OX_GICallState *cs, void **avalues, OX_Value *argsv)
{
    OX_GIArgType *atype;
    GIArgument *arg;
    OX_Result r;
    int aid_off = 0;

    if (cs->c->inst_bi) {
        void *avalue = avalues[0];
        void *p = *(void**)avalue;
        OX_Value *argv = ox_values_item(ctxt, argsv, 0);

        if (!p) {
            ox_value_set_null(ctxt, argv);
        } else {
            OX_GIOwn own = OX_GI_OWN_NOTHING;

            if ((r = giinst_to_value(ctxt, cs->gic.repo, p, &own, cs->c->inst_bi, argv)) == OX_ERR)
                goto end;
        }
        
        aid_off = 1;
    }

    for (atype = cs->c->in_atypes; atype; atype = atype->in_next) {
        int aid = atype - cs->c->atypes + aid_off;
        void *avalue = avalues[aid];

        arg = &cs->gic.in_args[atype->in_arg_idx];

        switch (gi_type_info_get_tag(atype->ti)) {
        case GI_TYPE_TAG_BOOLEAN:
            arg->v_boolean = *(gboolean*)avalue;
            break;
        case GI_TYPE_TAG_INT8:
            arg->v_int8 = *(int8_t*)avalue;
            break;
        case GI_TYPE_TAG_UINT8:
            arg->v_uint8 = *(uint8_t*)avalue;
            break;
        case GI_TYPE_TAG_INT16:
            arg->v_int16 = *(int16_t*)avalue;
            break;
        case GI_TYPE_TAG_UINT16:
            arg->v_uint16 = *(uint16_t*)avalue;
            break;
        case GI_TYPE_TAG_INT32:
            arg->v_int32 = *(int32_t*)avalue;
            break;
        case GI_TYPE_TAG_UINT32:
        case GI_TYPE_TAG_UNICHAR:
            arg->v_uint32 = *(uint32_t*)avalue;
            break;
        case GI_TYPE_TAG_INT64:
            arg->v_int64 = *(int64_t*)avalue;
            break;
        case GI_TYPE_TAG_UINT64:
            arg->v_uint64 = *(uint64_t*)avalue;
            break;
        case GI_TYPE_TAG_FLOAT:
            arg->v_float = *(float*)avalue;
            break;
        case GI_TYPE_TAG_DOUBLE:
            arg->v_double = *(double*)avalue;
            break;
        case GI_TYPE_TAG_GTYPE:
            arg->v_size = *(size_t*)avalue;
            break;
        default:
            arg->v_pointer = *(void**)avalue;
            break;
        }
    }

    for (atype = cs->c->in_atypes; atype; atype = atype->in_next) {
        OX_Value *argv = ox_values_item(ctxt, argsv, atype->in_arg_idx);
        GIArgument *arg = &cs->gic.in_args[atype->in_arg_idx];
        OX_GIOwn *own = &cs->in_owns[atype->in_arg_idx];

        if ((r = giargument_to_value(ctxt, &cs->gic, arg, own, atype->ti, argv)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    return r;
}

/*Store GIArgument to C pointer.*/
static void
gicallstate_arg_to_c (GITypeInfo *ti, GIArgument *arg, void *rvalue)
{
    switch (gi_type_info_get_tag(ti)) {
    case GI_TYPE_TAG_BOOLEAN:
        *(gboolean*)rvalue = arg->v_boolean;
        break;
    case GI_TYPE_TAG_INT8:
        *(int8_t*)rvalue = arg->v_int8;
        break;
    case GI_TYPE_TAG_UINT8:
        *(uint8_t*)rvalue = arg->v_uint8;
        break;
    case GI_TYPE_TAG_INT16:
        *(int16_t*)rvalue = arg->v_int16;
        break;
    case GI_TYPE_TAG_UINT16:
        *(uint16_t*)rvalue = arg->v_uint16;
        break;
    case GI_TYPE_TAG_INT32:
        *(int32_t*)rvalue = arg->v_int32;
        break;
    case GI_TYPE_TAG_UINT32:
    case GI_TYPE_TAG_UNICHAR:
        *(uint32_t*)rvalue = arg->v_uint32;
        break;
    case GI_TYPE_TAG_INT64:
        *(int64_t*)rvalue = arg->v_int64;
        break;
    case GI_TYPE_TAG_UINT64:
        *(uint64_t*)rvalue = arg->v_uint64;
        break;
    case GI_TYPE_TAG_FLOAT:
        *(float*)rvalue = arg->v_float;
        break;
    case GI_TYPE_TAG_DOUBLE:
        *(double*)rvalue = arg->v_double;
        break;
    case GI_TYPE_TAG_GTYPE:
        *(size_t*)rvalue = arg->v_size;
        break;
    default:
        *(void**)rvalue = arg->v_pointer;
        break;
    }
}

/*Convert output parameters form OX to C.*/
static OX_Result
gicallstate_output_c (OX_Context *ctxt, OX_GICallState *cs, void **avalues, void *rvalue, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, item)
    int ret_len = cs->c->n_out_args;
    OX_GIArgType *atype;
    OX_Result r;

    if (cs->c->ret_ti)
        ret_len ++;

    if (ret_len == 1) {
        if ((r = giargument_from_value(ctxt, &cs->gic, &cs->ret_arg, &cs->ret_own, cs->c->ret_ti, rv)) == OX_ERR)
            goto end;

        gicallstate_arg_to_c(cs->c->ret_ti, &cs->ret_arg, rvalue);
    } else if (ret_len > 1) {
        int i = 0;

        if (!ox_value_is_array(ctxt, rv)) {
            r = ox_throw_type_error(ctxt, OX_TEXT("the value is not an array"));
            goto end;
        }

        if (cs->c->ret_ti) {
            if ((r = ox_array_get_item(ctxt, rv, i, item)) == OX_ERR)
                goto end;

            if ((r = giargument_from_value(ctxt, &cs->gic, &cs->ret_arg, &cs->ret_own, cs->c->ret_ti, item)) == OX_ERR)
                goto end;

            i ++;
        }

        for (atype = cs->c->out_atypes; atype; atype = atype->out_next) {
            GIArgument *arg;
            OX_GIOwn *own;

            if (atype->dir == GI_DIRECTION_INOUT) {
                arg = &cs->gic.in_args[atype->in_arg_idx];
                own = &cs->in_owns[atype->in_arg_idx];
            } else {
                arg = &cs->gic.out_args[atype->out_arg_idx];
                own = &cs->out_owns[atype->out_arg_idx];
            }

            if ((r = ox_array_get_item(ctxt, rv, i, item)) == OX_ERR)
                goto end;

            if ((r = giargument_from_value(ctxt, &cs->gic, arg, own, atype->ti, item)) == OX_ERR)
                goto end;

            i ++;
        }

        if (cs->c->ret_ti)
            gicallstate_arg_to_c(cs->c->ret_ti, &cs->ret_arg, rvalue);

        for (atype = cs->c->out_atypes; atype; atype = atype->out_next) {
            int aid = atype - cs->c->atypes;
            GIArgument *arg;
            void *avalue;

            if (atype->dir == GI_DIRECTION_INOUT)
                arg = &cs->gic.in_args[atype->in_arg_idx];
            else
                arg = &cs->gic.out_args[atype->out_arg_idx];

            avalue = avalues[aid];

            gicallstate_arg_to_c(atype->ti, arg, avalue);
        }
    }

    if (cs->c->n_in_args)
        memset(cs->in_owns, 0, sizeof(OX_GIOwn) * cs->c->n_in_args);
    if (cs->c->n_out_args)
        memset(cs->out_owns, 0, sizeof(OX_GIOwn) * cs->c->n_out_args);

    cs->ret_own = OX_GI_OWN_NOTHING;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, item)
    return r;
}