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
 * Message queue.
 */

#define OX_LOG_TAG "queue"

#include "std.h"

/*Declaration index.*/
enum {
    ID_Queue,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "Queue",
    NULL
};

/*Script descrition.*/
static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/** Message queue.*/
typedef struct {
    OX_CondVar     cond;  /**< Condition variable.*/
    OX_ValueBuffer vb;    /**< Value buffer.*/
    size_t         start; /**< Start position.*/
    size_t         num;   /**< Number of message in the queue.*/
} OX_Queue;

/*Scan referenced objects in the message queue.*/
static void
queue_scan (OX_Context *ctxt, void *p)
{
    OX_Queue *q = p;
    size_t pos = q->start;
    size_t left = q->num;

    while (left) {
        OX_Value *v = q->vb.items + pos;

        ox_gc_scan_value(ctxt, v);

        pos ++;
        left --;

        if (pos >= q->vb.cap)
            pos = 0;
    }
}

/*Free the message queue.*/
static void
queue_free (OX_Context *ctxt, void *p)
{
    OX_Queue *q = p;

    ox_vector_deinit(ctxt, &q->vb);
    OX_DEL(ctxt, q);
}

/*Operation functions of the message queue.*/
static const OX_PrivateOps
queue_ops = {
    queue_scan,
    queue_free
};

/*Get the message queue from the value.*/
static OX_Queue*
queue_get (OX_Context *ctxt, OX_Value *v)
{
    OX_Queue *q = ox_object_get_priv(ctxt, v, &queue_ops);

    if (!q)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a queue"));

    return q;
}

/*Queue.$inf.$init*/
static OX_Result
Queue_inf_init (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Queue *q;
    OX_Result r;

    if (!OX_NEW(ctxt, q))
        return ox_throw_no_mem_error(ctxt);

    ox_cond_var_init(&q->cond);
    q->start = 0;
    q->num = 0;
    ox_vector_init(&q->vb);

    if ((r = ox_object_set_priv(ctxt, thiz, &queue_ops, q)) == OX_ERR) {
        queue_free(ctxt, q);
        return r;
    }

    return OX_OK;
}

/*Queue.$inf.send*/
static OX_Result
Queue_inf_send (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_Queue *q;
    size_t pos;
    OX_Result r;

    if (!(q = queue_get(ctxt, thiz)))
        return OX_ERR;

    if (q->num >= q->vb.cap) {
        if ((r = ox_vector_expand_capacity(ctxt, &q->vb, q->num + 1)) == OX_ERR)
            return ox_throw_no_mem_error(ctxt);
    }

    pos = (q->start + q->num) % q->vb.cap;
    ox_value_copy(ctxt, q->vb.items + pos, v);
    q->num ++;

    ox_cond_var_signal(&q->cond);

    return OX_OK;
}

/*Get a message from the queue.*/
static OX_Bool
queue_check (OX_Context *ctxt, OX_Queue *q, OX_Value *v)
{
    if (q->num) {
        size_t pos = q->start;

        ox_value_copy(ctxt, v, q->vb.items + pos);
        q->start ++;
        if (q->start == q->vb.cap)
            q->start = 0;
        q->num --;

        return OX_TRUE;
    } else {
        ox_value_set_null(ctxt, v);

        return OX_FALSE;
    }
}

/*Queue.$inf.wait*/
static OX_Result
Queue_inf_wait (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *time = ox_argument(ctxt, args, argc, 0);
    OX_VM *vm = ox_vm_get(ctxt);
    OX_Number n;
    int ms;
    OX_Queue *q;
    OX_Result r;

    if (!ox_value_is_null(ctxt, time)) {
        if ((r = ox_to_number(ctxt, time, &n)) == OX_ERR)
            return r;

        ms = n;
    } else {
        ms = -1;
    }

    if (!(q = queue_get(ctxt, thiz)))
        return OX_ERR;

    if (queue_check(ctxt, q, rv)) {
    } else if (ms == 0) {
        ox_value_set_null(ctxt, rv);
    } else {
        r = ox_cond_var_wait(&q->cond, &vm->lock, ms);
        if (r == OX_FALSE) {
            ox_value_set_null(ctxt, rv);
        } else if (r == OX_ERR) {
            return std_system_error_no(ctxt, "ox_cond_var_wait", r);
        } else {
            queue_check(ctxt, q, rv);
        }
    }

    return OX_OK;
}

/*Queue.$inf.check*/
static OX_Result
Queue_inf_check (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Queue *q;

    if (!(q = queue_get(ctxt, thiz)))
        return OX_ERR;

    queue_check(ctxt, q, rv);
    return OX_OK;
}

/*Load this module.*/
OX_Result
ox_load (OX_Context *ctxt, OX_Value *s)
{
    ox_not_error(ox_script_set_desc(ctxt, s, &script_desc));
    return OX_OK;
}

/*?
 *? @lib Message queue.
 *?
 *? @class{ Queue Message queue.
 *? Message queue is used for multi-thread communication.
 *? It has a internal FIFO queue in it to store the message values.
 *?
 *?
 *? @func $init Initialize the message queue object.
 *?
 *? @func send Send a message to the message queue.
 *? @param msg The message to be sent.
 *? The message will be store to the tail of the FIFO queue.
 *?
 *? @func wait Suspend current thread's execution and wait until there is any message in the queue.
 *? @param ms {Number} The waiting timeout time in milliseconds.
 *? @ul{
 *? @li If ms < 0, means the waiting time is infinite.
 *? @li If ms == 0, means only check the queue and do not wait.
 *? @ul}
 *? @return {?Any} If the queue receives a message within the timeout period, return the message at the beginning of the queue.
 *? If no message received, return null.
 *?
 *? @func check Check if the message queue has any message in it.
 *? @return {?Any} If the FIFO has any message in it, pop and return the message at the beginning of the queue.
 *? If no message is in the queue, return null.
 *?
 *? @class}
 */

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_2(ctxt, c, inf)

    /*Thread.*/
    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "Queue"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_Queue, c));

    /*Thread.$inf*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$init", Queue_inf_init));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "send", Queue_inf_send));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "wait", Queue_inf_wait));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "check", Queue_inf_check));

    OX_VS_POP(ctxt, c)
    return OX_OK;
}
