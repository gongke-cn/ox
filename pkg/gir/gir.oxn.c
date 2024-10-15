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
 * GIR module.
 */

#define OX_LOG_TAG "gir"

#include <girepository/girepository.h>
#include <girepository/girffi.h>

#define OX_TEXT_DOMAIN "gir"
#include <ox_internal.h>

/*Declaration index.*/
enum {
    ID_GIRepository,
    ID_GError,
    ID_GError_inf,
    ID_GSignalHandler_inf,
    ID_STR_domain,
    ID_STR_code,
    ID_STR_message,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "GIRepository",
    "GError",
    NULL
};

/** GI argument own flag.*/
typedef enum {
    OX_GI_OWN_NOTHING    = 0, /**< Own nothing.*/
    OX_GI_OWN_CONTAINER  = 1, /**< Only own the container.*/
    OX_GI_OWN_ELEMENT    = 2, /**< Own elements.*/
    OX_GI_OWN_EVERYTHING = 3  /**< Own container and the elements.*/
} OX_GIOwn;

/** GI argument type.*/
typedef struct OX_GIArgType_s OX_GIArgType;

/** GI argument type.*/
struct OX_GIArgType_s {
    GITypeInfo   *ti;          /**< Type information of the argument.*/
    OX_GIArgType *in_next;     /**< The next input argument type.*/
    OX_GIArgType *out_next;    /**< The next output argument type.*/
    int           in_arg_idx;  /**< Index in the input arguments.*/
    int           out_arg_idx; /**< Index in the output arguments.*/
    GIDirection   dir;         /**< Direction of the argument.*/
    OX_GIOwn      transfer;    /**< Ownership transfer.*/
};

/** GI repository.*/
typedef struct {
    GIRepository *repo;      /**< Repository.*/
    OX_Value      v;         /**< Repository value.*/
    OX_Value      script;    /**< The script contains this repository.*/
    OX_Hash       lib_hash;  /**< type library hash table.*/
    OX_Hash       type_hash; /**< Type hash table.*/
    OX_Hash       inst_hash; /**< Instance hash table.*/
    OX_Hash       cb_hash;   /**< Callback hash table.*/
} OX_GIRepository;

/** Call direction.*/
typedef enum {
    OX_CALL_C, /**< OX call C function.*/
    C_CALL_OX  /**< C call OX function.*/
} OX_GICallDir;

/** GI callable.*/
typedef struct {
    OX_GIRepository *repo;       /**< Repository.*/
    OX_GICallDir     dir;        /**< Call direction.*/
    OX_GIArgType    *atypes;     /**< Argument types.*/
    GIBaseInfo      *inst_bi;    /**< Instance's information.*/
    GITypeInfo      *ret_ti;     /**< Return value's type information.*/
    int              n_args;     /**< Number of arguments.*/
    int              n_in_args;  /**< Number of input arguments.*/
    int              n_out_args; /**< Nubmer of output arguments.*/
    gboolean         can_throw;  /**< The callable can throw a GError.*/
    OX_GIArgType    *in_atypes;  /**< Input argument types.*/
    OX_GIArgType    *out_atypes; /**< Output argument types.*/
    OX_GIOwn         inst_transfer; /**< Instant's ownership transfrer.*/
    OX_GIOwn         ret_transfer;  /**< Return value's ownership transfer.*/
} OX_GICallable;

/** GI context.*/
typedef struct {
    OX_GIRepository *repo;      /**< Repository.*/
    OX_GIArgType    *atypes;    /**< Argument types.*/
    GIArgument      *in_args;   /**< Input arguments.*/
    GIArgument      *out_args;  /**< Output arguments.*/
} OX_GICtxt;

/** GI parameters.*/
typedef struct {
    OX_Context  *ctxt; /**< The context.*/
    OX_GICtxt   *gic;  /**< GI context.*/
    GITypeInfo  *ti0;  /**< The type information 0.*/
    GITypeInfo  *ti1;  /**< The type information 1.*/
    OX_Value    *v;    /**< The value.*/
    OX_Result    r;    /**< Result.ss*/
} OX_GIParams;

/*Get the type's size.*/
static size_t
gitype_get_size (OX_Context *ctxt, OX_GICtxt *gic, GITypeInfo *ti);
/*Get the element size of a container.*/
static size_t
gcontainer_get_element_size (OX_Context *ctxt, OX_GICtxt *gic, GITypeInfo *ti);
/*Get the length of the array.*/
static size_t
array_get_length (OX_Context *ctxt, OX_GICtxt *gic, GITypeInfo *ti);
/*Free the argument data.*/
static void
giargument_free (OX_Context *ctxt, OX_GICtxt *gic, GIArgument *arg, OX_GIOwn own, GITypeInfo *ti);
/*Convert the argument to OX value.*/
static OX_Result
giargument_to_value (OX_Context *ctxt, OX_GICtxt *gic, GIArgument *arg, OX_GIOwn *pown, GITypeInfo *ti, OX_Value *v);
/*Convert the OX value to GIArgument.*/
static OX_Result
giargument_from_value (OX_Context *ctxt, OX_GICtxt *gic, GIArgument *arg, OX_GIOwn *pown, GITypeInfo *ti, OX_Value *v);
/*Convert the base information to value.*/
static OX_Result
gibaseinfo_to_value (OX_Context *ctxt, OX_GIRepository *repo, GIBaseInfo *bi, OX_Value *v);
/*Convert the base information to the object's constance property.*/
static OX_Result
gibaseinfo_to_const_prop (OX_Context *ctxt, OX_GIRepository *repo, GIBaseInfo *bi, OX_Value *o, OX_Value *v);
/*Convert callback to OX value.*/
static OX_Result
gicallback_to_value (OX_Context *ctxt, OX_GIRepository *repo, void *p, GICallableInfo *ci, OX_Value *v);
/*Create a new callback.*/
static OX_Result
gicallback_from_value (OX_Context *ctxt, OX_GIRepository *repo, OX_Value *v, GICallableInfo *ci, void **pp);

#include "gictxt.h"
#include "gitype.h"
#include "giinst.h"
#include "gerror.h"
#include "gcontainer.h"
#include "glist.h"
#include "gslist.h"
#include "ghash.h"
#include "array.h"
#include "giargument.h"
#include "giconstantinfo.h"
#include "gifieldinfo.h"
#include "gicallable.h"
#include "gicallback.h"
#include "givfuncinfo.h"
#include "gifunctioninfo.h"
#include "gisignal.h"
#include "gipropertyinfo.h"
#include "giinterfaceinfo.h"
#include "giobjectinfo.h"
#include "gistructinfo.h"
#include "giunioninfo.h"
#include "gienuminfo.h"
#include "gibaseinfo.h"
#include "gitypelib.h"
#include "girepository.h"

/*Script description.*/
static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/*Load this module.*/
OX_Result
ox_load (OX_Context *ctxt, OX_Value *s)
{
    ox_not_error(ox_script_set_desc(ctxt, s, &script_desc));
    return OX_OK;
}

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    gerror_class_init(ctxt, s);
    gisignalhandler_class_init(ctxt, s);
    girepository_class_init(ctxt, s);

    return OX_OK;
}