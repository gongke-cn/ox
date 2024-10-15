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
 * Virtual machine.
 */

/*?
 *? @package builtin OX built-in modules.
 *? All the built-in modules will be loaded automatically.
 *? OX scripts can invoke the functions and classes in the built-in modules directly.
 */

#define OX_LOG_TAG "ox_vm"

#include "ox_internal.h"
#include "ox_string_table.h"

/*Initialize the context.*/
static void
context_init (OX_Context *ctxt, OX_VM *vm)
{
    ctxt->base.v_stack = &ctxt->bot_v_stack;
    ctxt->base.vm = vm;
    ctxt->base.sched_cnt = 0;
    ctxt->base.priv = NULL;

    ox_thread_key_set(&vm->key, ctxt);

    ctxt->s_stack = &ctxt->bot_s_stack;
    ctxt->curr_script = NULL;
    ctxt->frames = NULL;
    ctxt->error_frames = NULL;
    ctxt->main_frames = NULL;
    ctxt->lock_cnt = 0;

    ox_value_set_null(ctxt, &ctxt->error);
    ox_vector_init(&ctxt->bot_v_stack);
    ox_vector_init(&ctxt->bot_s_stack);

    ox_list_append(&vm->ctxt_list, &ctxt->ln);

    vm->ref ++;
}

/*Release the context.*/
static void
context_deinit (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);
    size_t i;

    ox_list_remove(&ctxt->ln);

    /*Clear the stack.*/
    for (i = 0; i < ctxt->bot_s_stack.len; i ++) {
        OX_Stack *se = &ox_vector_item(&ctxt->bot_s_stack, i);

        ox_stack_deinit(ctxt, se);
    }

    ox_vector_deinit(ctxt, &ctxt->bot_s_stack);
    ox_vector_deinit(ctxt, &ctxt->bot_v_stack);

    vm->ref --;
}

/*Free the virtual machine.*/
static void
vm_free (OX_VM *vm)
{
    OX_Context *ctxt;
    size_t i;
    OX_GlobalRef *ref, *nref;

    /*Get the main context.*/
    ctxt = ox_thread_key_get(&vm->key);
    assert(ctxt);

    context_deinit(ctxt);
    ox_thread_key_set(&vm->key, NULL);

    /*Clear the global reference hash table.*/
    ox_hash_foreach_safe_c(&vm->global_ref_hash, i, ref, nref, OX_GlobalRef, he) {
        OX_DEL(ctxt, ref);
    }
    ox_hash_deinit(ctxt, &vm->global_ref_hash);

    /*Free the resource in the virtual macine.*/
    ox_script_hash_deinit(ctxt);
    ox_gc_deinit(ctxt);
    ox_string_singleton_deinit(ctxt);

    /*File encoding.*/
    if (vm->file_enc)
        ox_strfree(ctxt, vm->file_enc);

    /*Installation directory.*/
    if (vm->install_dir)
        ox_strfree(ctxt, vm->install_dir);

    ox_package_deinit(ctxt);
    ox_mem_deinit(ctxt);
    ox_log_deinit(ctxt);

    /*Free the main context.*/
    free(ctxt);

    /*Free the virtual machine.*/
    ox_mutex_deinit(&vm->lock);
    ox_thread_key_deinit(&vm->key);

    free(vm);
}

/**
 * Free the context.
 * @param ctxt The context to be freed.
 */
void
ox_context_free (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);
    OX_Bool can_free;

    ox_mutex_lock(&vm->lock);
    context_deinit(ctxt);
    can_free = (vm->ref == 0);
    ox_mutex_unlock(&vm->lock);

    free(ctxt);

    if (can_free)
        vm_free(vm);
}

/*Create internal strings.*/
static void
create_strings (OX_Context *ctxt)
{
    size_t id;

    for (id = 0; id < OX_STR_ID_MAX; id ++) {
        OX_Value *v = &ox_vm_get(ctxt)->strings[id];
        const char *s = string_table[id];

        ox_not_error(ox_string_from_const_char_star(ctxt, v, s));
        ox_not_error(ox_string_singleton(ctxt, v));
    }
}

/**
 * Create a new virtual machine.
 * @return The new virtual machine.
 */
OX_VM*
ox_vm_new (void)
{
    OX_VM *vm;
    OX_Context *ctxt;

    /*Allocate the virtual machine and the main conext.*/
    vm = malloc(sizeof(OX_VM));
    assert(vm);

    vm->ref = 0;

    ox_mutex_init(&vm->lock);
    ox_thread_key_init(&vm->key);
    ox_list_init(&vm->ctxt_list);

    ctxt = malloc(sizeof(OX_Context));
    assert(ctxt);

    context_init(ctxt, vm);

    /*Global reference hash table.*/
    ox_size_hash_init(&vm->global_ref_hash);

    /*Initialize the virtual machine.*/
    ox_log_init(ctxt);
    ox_mem_init(ctxt);
    ox_gc_init(ctxt);
    ox_string_singleton_init(ctxt);
    ox_script_hash_init(ctxt);

    /*File encoding.*/
    vm->file_enc = NULL;
    /*Installation directory.*/
    vm->install_dir = NULL;

    /*Dump stack when throw an error.*/
    vm->dump_throw = OX_FALSE;

    ox_values_set_null(ctxt, vm->strings, OX_STR_ID_MAX);
    ox_values_set_null(ctxt, vm->objects, OX_OBJ_ID_MAX);
    ox_value_set_null(ctxt, &vm->base.v_null);
    ox_value_set_null(ctxt, &vm->packages);

    /*Create strings.*/
    create_strings(ctxt);

    /*Create global objects.*/
    ox_not_error(ox_object_new(ctxt, OX_OBJECT(ctxt, Global), NULL));
    ox_func_class_init(ctxt);
    ox_iterator_class_init(ctxt);
    ox_bool_class_init(ctxt);
    ox_number_class_init(ctxt);
    ox_string_class_init(ctxt);
    ox_array_class_init(ctxt);
    ox_object_class_init(ctxt);
    ox_error_class_init(ctxt);
    ox_script_object_init(ctxt);
    ox_enum_object_init(ctxt);
    ox_re_class_init(ctxt);
    ox_match_class_init(ctxt);
    ox_set_class_init(ctxt);
    ox_dict_class_init(ctxt);
    ox_ctype_class_init(ctxt);
    ox_proxy_class_init(ctxt);
    ox_ast_class_init(ctxt);

    /*Package manager.*/
    ox_package_init(ctxt);

    return vm;
}

/**
 * Free the unused virtual machine.
 * @param vm The virtual machine to be freed.
 */
void
ox_vm_free (OX_VM *vm)
{
    OX_Bool can_free;

    assert(vm);

    ox_mutex_lock(&vm->lock);
    assert(vm->ref > 0);
    vm->ref --;
    can_free = (vm->ref == 0);
    ox_mutex_unlock(&vm->lock);

    if (can_free)
        vm_free(vm);
}

/**
 * Get the current running context.
 * @param vm The virtual machine contains this context.
 * @return The new running context.
 */
OX_Context*
ox_context_get (OX_VM *vm)
{
    OX_Context *ctxt;

    assert(vm);

    ctxt = ox_thread_key_get(&vm->key);
    if (!ctxt) {
        ctxt = malloc(sizeof(OX_Context));
        assert(ctxt);

        ox_mutex_lock(&vm->lock);
        context_init(ctxt, vm);
        ox_mutex_unlock(&vm->lock);
    }

    return ctxt;
}

/**
 * Get the OX version number.
 * @return The version number.
 */
const char*
ox_get_version (void)
{
    return VERSION;
}

/**
 * Get the OX linker flags.
 * @return The linker flags. 
 */
const char*
ox_get_ld_flags (void)
{
    return LIBS;
}
