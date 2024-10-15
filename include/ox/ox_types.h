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
 * Basic data types.
 */

#ifndef _OX_TYPES_H_
#define _OX_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/** Boolean value.*/
typedef uint8_t OX_Bool;

/** Boolean value: true.*/
#define OX_TRUE  1

/** Boolean value: false.*/
#define OX_FALSE 0

/** Function result.*/
typedef int OX_Result;

/** Function result: OK.*/
#define OX_OK 1

/** Function result: error.*/
#define OX_ERR -1

/** Number.*/
typedef double OX_Number;

/** Maximum safe integer number.*/
#define OX_MAX_SAFE_INTEGER 9007199254740991
/** Minimum safe integer number.*/
#define OX_MIN_SAFE_INTEGER -9007199254740991

/**Log level.*/
typedef int OX_LogLevel;

#define OX_LOG_LEVEL_ALL     0 /**< Output all message.*/
#define OX_LOG_LEVEL_DEBUG   1 /**< Debug message.*/
#define OX_LOG_LEVEL_INFO    2 /**< Normal information.*/
#define OX_LOG_LEVEL_WARNING 3 /**< Warning message.*/
#define OX_LOG_LEVEL_ERROR   4 /**< Error message.*/
#define OX_LOG_LEVEL_FATAL   5 /**< Fatal error message.*/
#define OX_LOG_LEVEL_NONE    6 /**< Not output any message.*/

/** Log output field.*/
typedef enum {
    OX_LOG_FIELD_TAG    = (1 << 0), /**< Load tag.*/
    OX_LOG_FIELD_LEVEL  = (1 << 1), /**< Log level.*/
    OX_LOG_FIELD_DATE   = (1 << 2), /**< Date.*/
    OX_LOG_FIELD_TIME   = (1 << 3), /**< Time.*/
    OX_LOG_FIELD_MSEC   = (1 << 4), /**< Millieconds.*/
    OX_LOG_FIELD_FILE   = (1 << 5), /**< Filename.*/
    OX_LOG_FIELD_FUNC   = (1 << 6), /**< Function name.*/
    OX_LOG_FIELD_LINE   = (1 << 7), /**< Line number.*/
    OX_LOG_FIELD_THREAD = (1 << 8), /**< Thread ID.*/
    OX_LOG_FIELD_ALL    = 0xffffffff/**< Output all field.*/
} OX_LogField;

/** Prompt message type.*/
typedef enum {
    OX_PROMPT_NOTE,    /**< Note message.*/
    OX_PROMPT_WARNING, /**< Warning message.*/
    OX_PROMPT_ERROR    /**< Error message.*/
} OX_PromptType;

/** Location.*/
typedef struct {
    int first_line;   /**< First character's line number.*/
    int first_column; /**< First character's column number.*/
    int last_line;    /**< Last character's line number.*/
    int last_column;  /**< Last character's column number.*/
} OX_Location;

/** Virtual machine.*/
typedef struct OX_VM_s OX_VM;
/** Running context.*/
typedef struct OX_Context_s OX_Context;
/** Character input.*/
typedef struct OX_Input_s OX_Input;

/** List node.*/
typedef struct OX_List_s OX_List;
/** List node.*/
struct OX_List_s {
    OX_List *prev; /**< The previous node in the list.*/
    OX_List *next; /**< The next node in the list.*/
};

/**
 * Declare a vector type.
 * @param t The element type.
 */
#define OX_VECTOR_TYPE_DECL(t)\
    struct {\
        t *items;\
        size_t len;\
        size_t cap;\
    }

/** Character buffer.*/
typedef OX_VECTOR_TYPE_DECL(char) OX_CharBuffer;

/** Hash table entry.*/
typedef struct OX_HashEntry_s OX_HashEntry;
/** Hash table entry.*/
struct OX_HashEntry_s {
    OX_HashEntry *next; /**< The next entry in the list.*/
    void         *key;  /**< Key of the entry.*/
};

/** Hash table.*/
typedef struct OX_Hash_s OX_Hash;

/** Hash table operation functions.*/
typedef struct {
    /** Key code calculate function.*/
    size_t  (*key) (OX_Context *ctxt, void *k);
    /** Key equal check function.*/
    OX_Bool (*equal) (OX_Context *ctxt, void *k1, void *k2);
} OX_HashOps;

/** Hash table.*/
struct OX_Hash_s {
    const OX_HashOps *ops;    /**< Operation functions.*/
    union {
        OX_HashEntry  *list;  /**< Entry list.*/
        OX_HashEntry **lists; /**< Entry list array.*/
    } e;                      /**< Entries data.*/
    size_t e_num; /**< Number of entries.*/
    size_t l_num; /**< Number of the entries' list.*/
};

/** The object has generic operation functions.*/
#define OX_GCO_FL_OPS    (1 << 0)
/** The object is an generic object.*/
#define OX_GCO_FL_OBJECT (1 << 1)
/** The object is a function.*/
#define OX_GCO_FL_FUNC   (1 << 2)
/** The object is an input.*/
#define OX_GCO_FL_INPUT  (1 << 3)
/** The object is a script.*/
#define OX_GCO_FL_SCRIPT (1 << 4)
/** The object is a string.*/
#define OX_GCO_FL_STRING (1 << 5)

/** Make the GC managed object type.*/
#define OX_GCO_TYPE(t, f) (((t) << 8) | (f))

/** GC managed object type.*/
typedef enum {
    OX_GCO_STRING_INPUT = OX_GCO_TYPE(0, OX_GCO_FL_INPUT),  /**< String input.*/
    OX_GCO_FILE_INPUT   = OX_GCO_TYPE(1, OX_GCO_FL_INPUT),  /**< File input.*/
    OX_GCO_STRING       = OX_GCO_TYPE(2, OX_GCO_FL_OPS|OX_GCO_FL_STRING), /**< String.*/
    OX_GCO_SINGLETON_STRING = OX_GCO_TYPE(3, OX_GCO_FL_OPS|OX_GCO_FL_STRING), /**< Singleton string.*/
    OX_GCO_OBJECT       = OX_GCO_TYPE(4, OX_GCO_FL_OPS|OX_GCO_FL_OBJECT), /**< Object.*/
    OX_GCO_ARRAY        = OX_GCO_TYPE(5, OX_GCO_FL_OPS|OX_GCO_FL_OBJECT), /**< Array.*/
    OX_GCO_FUNCTION     = OX_GCO_TYPE(6, OX_GCO_FL_OPS|OX_GCO_FL_OBJECT|OX_GCO_FL_FUNC), /**< Function.*/
    OX_GCO_NATIVE_FUNC  = OX_GCO_TYPE(7, OX_GCO_FL_OPS|OX_GCO_FL_OBJECT|OX_GCO_FL_FUNC), /**< Native function.*/
    OX_GCO_INTERFACE    = OX_GCO_TYPE(8, OX_GCO_FL_OPS|OX_GCO_FL_OBJECT), /**< Interface.*/
    OX_GCO_FRAME        = OX_GCO_TYPE(9, 0),                /**< Value frame.*/
    OX_GCO_SCRIPT       = OX_GCO_TYPE(10, OX_GCO_FL_OPS|OX_GCO_FL_SCRIPT), /**< Script.*/
    OX_GCO_BC_SCRIPT    = OX_GCO_TYPE(11,OX_GCO_FL_OPS|OX_GCO_FL_SCRIPT), /**< Byte code script.*/
    OX_GCO_NATIVE_SCRIPT= OX_GCO_TYPE(12,OX_GCO_FL_OPS|OX_GCO_FL_SCRIPT), /**< Native script.*/
    OX_GCO_CLASS        = OX_GCO_TYPE(13,OX_GCO_FL_OPS|OX_GCO_FL_OBJECT), /**< Class.*/
    OX_GCO_ENUM         = OX_GCO_TYPE(14,OX_GCO_FL_OPS|OX_GCO_FL_OBJECT), /**< Enumeration.*/
    OX_GCO_RE           = OX_GCO_TYPE(15,OX_GCO_FL_OPS|OX_GCO_FL_OBJECT), /**< Regular expression.*/
    OX_GCO_MATCH        = OX_GCO_TYPE(16,OX_GCO_FL_OPS|OX_GCO_FL_OBJECT), /**< Match result.*/
    OX_GCO_SET          = OX_GCO_TYPE(17,OX_GCO_FL_OPS|OX_GCO_FL_OBJECT), /**< Data set.*/
    OX_GCO_DICT         = OX_GCO_TYPE(18,OX_GCO_FL_OPS|OX_GCO_FL_OBJECT), /**< Dictionary.*/
    OX_GCO_CTYPE        = OX_GCO_TYPE(19,OX_GCO_FL_OPS|OX_GCO_FL_OBJECT), /**< C type.*/
    OX_GCO_CVALUE       = OX_GCO_TYPE(20,OX_GCO_FL_OPS|OX_GCO_FL_OBJECT), /**< C value.*/
    OX_GCO_PROXY        = OX_GCO_TYPE(21,OX_GCO_FL_OPS|OX_GCO_FL_OBJECT), /**< Proxy object.*/
} OX_GcObjectType;

/** GC managed object.*/
typedef struct OX_GcObject_s OX_GcObject;

/** Operation functions of GC managed object.*/
typedef struct {
    OX_GcObjectType type; /**< Type of the object.*/
    /** Scan the referenced objects.*/
    void (*scan) (OX_Context *ctxt, OX_GcObject *gco);
    /** Free the GC managed object.*/
    void (*free) (OX_Context *ctxt, OX_GcObject *gco);
} OX_GcObjectOps;

/** The GC managed object is marked.*/
#define OX_GC_FL_MARKED  (1 << 0)
/** The GC managed object is scanned.*/
#define OX_GC_FL_SCANNED (1 << 1)

/** GC managed object.*/
struct OX_GcObject_s {
    const OX_GcObjectOps *ops; /**< Operation functions.*/
    size_t                next_flags; /**< The next GC managed object and flags.*/
};

/** String.*/
typedef struct {
    OX_GcObject gco;   /**< Base GC managed object data.*/
    char       *chars; /**< Characters buffer.*/
    size_t      len;   /**< Length of the string.*/
} OX_String;

/** String trim mode.*/
typedef enum {
    OX_STRING_TRIM_HEAD = 1, /**< Trim the head of the string.*/
    OX_STRING_TRIM_TAIL = 2, /**< Trim the tail of the string.*/
    OX_STRING_TRIM_BOTH = (OX_STRING_TRIM_HEAD|OX_STRING_TRIM_TAIL) /**< Trim the head and tail of the string.*/
} OX_StringTrim;

/**
 * Value.
 *
 * The value should be in heap or stack.
 * If the value is in heap, the value pointer is its real address.
 * If the value is in stack, the value pointer is ((stack_index << 1) | 1).
 */
typedef uint64_t OX_Value;

/** Value buffer.*/
typedef OX_VECTOR_TYPE_DECL(OX_Value) OX_ValueBuffer;

/** Value type.*/
typedef enum {
    OX_VALUE_NULL,   /**< null.*/
    OX_VALUE_BOOL,   /**< Boolean value.*/
    OX_VALUE_NUMBER, /**< Number value.*/
    OX_VALUE_GCO     /**< GC managed object.*/
} OX_ValueType;

/** Operation functions of the object.*/
typedef struct {
    /** Base GC managed object operation functions.*/
    OX_GcObjectOps gco_ops;
    /** Get the keys of the object.*/
    OX_Result (*keys) (OX_Context *ctxt, OX_Value *o, OX_Value *keys);
    /** Lookup the owned property.*/
    OX_Result (*lookup) (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v);
    /** Get property value.*/
    OX_Result (*get) (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v);
    /** Set property value.*/
    OX_Result (*set) (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v);
    /** Delete a property.*/
    OX_Result (*del) (OX_Context *ctxt, OX_Value *o, OX_Value *p);
    /** Call the object.*/
    OX_Result (*call) (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *r);
} OX_ObjectOps;

/** Property type.*/
typedef enum {
    OX_PROPERTY_VAR,      /**< Variable.*/
    OX_PROPERTY_CONST,    /**< Constant field.*/
    OX_PROPERTY_ACCESSOR  /**< Accessor.*/
} OX_PropertyType;

/** Property.*/
typedef struct {
    OX_PropertyType  type; /**< Property type.*/
    OX_List          ln;   /**< List node data.*/
    OX_HashEntry     he;   /**< Hash table entry data.*/
    union {
        struct {
            OX_Value get;  /**< Getter of accessor.*/
            OX_Value set;  /**< Setter of accessor.*/
        } a;               /**< Accessor data.*/
        OX_Value     v;    /**< Value.*/
    } p;                   /**< Property data.*/
} OX_Property;

/** Private data operation functions.*/
typedef struct {
    /** Scan referenced objects function of the private data.*/
    void (*scan) (OX_Context *ctxt, void *ptr);
    /** Free function of the private data.*/
    void (*free) (OX_Context *ctxt, void *ptr);
} OX_PrivateOps;

/** Object.*/
typedef struct OX_Object_s OX_Object;
/** Object.*/
struct OX_Object_s {
    OX_GcObject  gco;     /**< Base GC managed object data.*/
    OX_Value     inf;     /**< Interface object.*/
    OX_List      p_list;  /**< Properties list.*/
    OX_Hash      p_hash;  /**< Properties hash table.*/
    const OX_PrivateOps *priv_ops; /**< Private data's operation functions.*/
    void        *priv;    /**< Private data.*/
};

/** Object iterator's type.*/
typedef enum {
    OX_OBJECT_ITER_KEY,      /**< Property key only.*/
    OX_OBJECT_ITER_VALUE,    /**< Property value only.*/
    OX_OBJECT_ITER_KEY_VALUE /**< Property key and value.*/
} OX_ObjectIterType;

/** Interface.*/
typedef struct {
    OX_Object o;      /**< Base object data.*/
    OX_Hash   i_hash; /**< Inherited interface hash table.*/
} OX_Interface;

/** Allocate a new object function/*/
typedef OX_Result (*OX_AllocObjectFunc) (OX_Context *ctxt, OX_Value *o, OX_Value *inf);

/** Class.*/
typedef struct {
    OX_Object          o;     /**< Base object data.*/
    OX_AllocObjectFunc alloc; /**< Allocate object function.*/
} OX_Class;

/** Array.*/
typedef struct {
    OX_Object o; /**< Base object data.*/
    OX_VECTOR_TYPE_DECL(OX_Value) items; /**< Items of the array.*/
} OX_Array;

/**
 * C function.
 * @param ctxt The current running context.
 * @param f The native function.
 * @param thiz This argument.
 * @param args Arguments.
 * @param argc Arguments' count.
 * @param r The return value.
 */
typedef OX_Result (*OX_CFunc) (
        OX_Context *ctxt,
        OX_Value *f,
        OX_Value *thiz,
        OX_Value *args,
        size_t argc,
        OX_Value *r);

/** Operation functions of character input.*/
typedef struct {
    /** Base GC managed object operation functions.*/
    OX_GcObjectOps gco_ops;
    /** Get an unicode character from the input.*/
    int  (*get_char) (OX_Context *ctxt, OX_Input *input);
    /** Push back an unicode character to the input.*/
    void (*unget_char) (OX_Context *ctxt, OX_Input *input, int c);
    /** Get the read position.*/
    long (*tell) (OX_Context *ctxt, OX_Input *input);
    /** Close the input.*/
    void (*close) (OX_Context *ctxt, OX_Input *input);
    /** Reopen the input.*/
    OX_Result (*reopen) (OX_Context *ctxt, OX_Input *input, OX_Value *v, long off);
} OX_InputOps;

/** The input is closed.*/
#define OX_INPUT_ST_CLOSED   (1 << 0)
/** The last character is newline.*/
#define OX_INPUT_ST_LF       (1 << 1)
/** The input has error.*/
#define OX_INPUT_ST_ERR      (1 << 2)
/** No not show the input text.*/
#define OX_INPUT_ST_NOT_SHOW (1 << 3)

/** The input is end.*/
#define OX_INPUT_END (-1)

/** Input location stub.*/
typedef struct {
    int  line;   /**< Line number.*/
    int  column; /**< Column number.*/
    long offset; /**< File offset.*/
} OX_InputLocStub;

/** Character input.*/
struct OX_Input_s {
    OX_GcObject gco;     /**< Base GC managed object data.*/
    char       *name;    /**< Name of the input.*/
    int         status;  /**< The status of the input.*/
    int         line;    /**< The current line number.*/
    int         column;  /**< The current column number.*/
    size_t      counter; /**< Character counter.*/
    OX_VECTOR_TYPE_DECL(OX_InputLocStub) loc_stubs; /**< Location stubs.*/
};

/** Reference description.*/
typedef struct {
    char *script; /**< The script's name.*/
    char *orig;   /**< Origin name.*/
    char *local;  /**< Local name.*/
} OX_RefDesc;

/** Script description.*/
typedef struct {
    const OX_RefDesc *refs;       /**< Reference entries.*/
    const char      **pubs;       /**< Public entries.*/
    size_t            frame_size; /**< The frame size.*/
} OX_ScriptDesc;

/** Enumeration type.*/
typedef enum {
    OX_ENUM_ENUM,     /**< Enumeration.*/
    OX_ENUM_BITFIELD  /**< Bitfield.*/
} OX_EnumType;

/** Regular expression's flag.*/
typedef enum {
    OX_RE_FL_IGNORE_CASE = (1 << 0), /**< Ignore case.*/
    OX_RE_FL_MULTILINE   = (1 << 1), /**< Multiline.*/
    OX_RE_FL_DOT_ALL     = (1 << 2), /**< "." can match newline.*/
    OX_RE_FL_UNICODE     = (1 << 3), /**< Unicode match mode.*/
    OX_RE_FL_PERFECT     = (1 << 4)  /**< Perfect match mode.*/
} OX_ReFlag;

/** Regular expression command type.*/
typedef enum {
    OX_RE_CMD_NEXT,       /**< Get the next character.*/
    OX_RE_CMD_PREV,       /**< Get the previous character.*/
    OX_RE_CMD_MATCH_ALL,  /**< Match all character.*/
    OX_RE_CMD_MATCH_CHAR, /**< Match a character.*/
    OX_RE_CMD_MATCH_RANGE,/**< Match character range.*/
    OX_RE_CMD_MATCH_LS,   /**< Match line start.*/
    OX_RE_CMD_MATCH_LE,   /**< Match line end.*/
    OX_RE_CMD_MATCH_S,    /**< Match a space character.*/
    OX_RE_CMD_MATCH_NS,   /**< Match a character not space.*/
    OX_RE_CMD_MATCH_D,    /**< Match a digit character.*/
    OX_RE_CMD_MATCH_ND,   /**< Match a character not digit.*/
    OX_RE_CMD_MATCH_W,    /**< Match a word character.*/
    OX_RE_CMD_MATCH_NW,   /**< Match a character not word.*/
    OX_RE_CMD_MATCH_B,    /**< Match the blank position.*/
    OX_RE_CMD_MATCH_NB,   /**< Match the position not blank.*/
    OX_RE_CMD_MATCH_BR,   /**< Back reference.*/
    OX_RE_CMD_GROUP_START,/**< Group start.*/
    OX_RE_CMD_GROUP_END,  /**< Group end.*/
    OX_RE_CMD_PUSH,       /**< Push the state.*/
    OX_RE_CMD_POP,        /**< Pop up the state from the stack.*/
    OX_RE_CMD_PUSH_POS,   /**< Push the position.*/
    OX_RE_CMD_POP_POS,    /**< Pop up the position from the stack.*/
    OX_RE_CMD_JMP,        /**< Jump to the command.*/
    OX_RE_CMD_ACCEPT,     /**< Accept.*/
    OX_RE_CMD_REJECT      /**< Reject.*/
} OX_ReCmdType;

/** Regular expression command.*/
typedef struct {
    OX_ReCmdType type; /**< Command type.*/
    int          l;    /**< Jump destination when mismatch.*/
    union {
        int      c;    /**< Character.*/
        int      br;   /**< Back reference index.*/
        int      gid;  /**< Group index.*/
        struct {
            int  min;  /**< Minimum character.*/
            int  max;  /**< Maximum character.*/
        } r;           /**< Character range.*/
    } c;               /**< Command data.*/
} OX_ReCmd;

/** Regular expression.*/
typedef struct {
    OX_Object  o;         /**< Base object data.*/
    OX_ReFlag  flags;     /**< Flags.*/
    int        group_num; /**< Groups number in the regular expression.*/
    OX_ReCmd  *cmds;      /**< Commands buffer.*/
    size_t     cmd_len;   /**< Commands buffer's length.*/
    OX_Value   src;       /**< Source of the regular expression.*/
} OX_Re;

/** Make a C type kind.*/
#define OX_CTYPE_KIND(f, n) ((f) | (n))

/** Number.*/
#define OX_CTYPE_FL_NUM   (1 << 8)
/** Integer number.*/
#define OX_CTYPE_FL_INT   ((1 << 9) | OX_CTYPE_FL_NUM)
/** Float point number.*/
#define OX_CTYPE_FL_FLOAT ((1 << 10) | OX_CTYPE_FL_NUM)
/** Store the pointer of the data.*/
#define OX_CTYPE_FL_PTR   (1 << 11)

/** Kind of the C type.*/
typedef enum {
    OX_CTYPE_I8     = OX_CTYPE_KIND(OX_CTYPE_FL_INT, 0), /**< 8 bits integer.*/
    OX_CTYPE_U8     = OX_CTYPE_KIND(OX_CTYPE_FL_INT, 1), /**< 8 bits unsigned integer.*/
    OX_CTYPE_I16    = OX_CTYPE_KIND(OX_CTYPE_FL_INT, 2), /**< 16 bits integer.*/
    OX_CTYPE_U16    = OX_CTYPE_KIND(OX_CTYPE_FL_INT, 3), /**< 16 bits unsigned integer.*/
    OX_CTYPE_I32    = OX_CTYPE_KIND(OX_CTYPE_FL_INT, 4), /**< 32 bits integer.*/
    OX_CTYPE_U32    = OX_CTYPE_KIND(OX_CTYPE_FL_INT, 5), /**< 32 bits unsigned integer.*/
    OX_CTYPE_I64    = OX_CTYPE_KIND(OX_CTYPE_FL_INT, 6), /**< 64 bits integer.*/
    OX_CTYPE_U64    = OX_CTYPE_KIND(OX_CTYPE_FL_INT, 7), /**< 64 bits unsigned integer.*/
    OX_CTYPE_F32    = OX_CTYPE_KIND(OX_CTYPE_FL_NUM, 8), /**< 32 bits float point number.*/
    OX_CTYPE_F64    = OX_CTYPE_KIND(OX_CTYPE_FL_NUM, 9), /**< 64 bits float point number.*/
    OX_CTYPE_STR    = OX_CTYPE_KIND(OX_CTYPE_FL_PTR, 10),/**< 0 terminated string.*/
    OX_CTYPE_PTR    = OX_CTYPE_KIND(OX_CTYPE_FL_PTR, 11),/**< Pointer.*/
    OX_CTYPE_ARRAY  = OX_CTYPE_KIND(OX_CTYPE_FL_PTR, 12),/**< Array.*/
    OX_CTYPE_FUNC   = OX_CTYPE_KIND(OX_CTYPE_FL_PTR, 13),/**< Function.*/
    OX_CTYPE_STRUCT = OX_CTYPE_KIND(OX_CTYPE_FL_PTR, 14),/**< Structure.*/
    OX_CTYPE_VOID   = OX_CTYPE_KIND(0, 15)               /**< Void.*/
} OX_CTypeKind;

#if __SIZEOF_POINTER__  == 8
    #define OX_CTYPE_SIZE  OX_CTYPE_U64
    #define OX_CTYPE_SSIZE OX_CTYPE_I64
#elif __SIZEOF_POINTER__  == 4
    #define OX_CTYPE_SIZE  OX_CTYPE_U32
    #define OX_CTYPE_SSIZE OX_CTYPE_I32
#else
    #error illegal pointer size
#endif

/** C type data.*/
typedef struct {
    OX_Object    o;    /**< Base object data.*/
    OX_CTypeKind kind; /**< Kind of the C type.*/
    ssize_t      size; /**< Size of the type in bytes.*/
    OX_Value     pty;  /**< Pointer of this type.*/
    OX_Value     aty;  /**< Array of this type.*/
} OX_CType;

/** C pointer type.*/
typedef struct {
    OX_CType  cty; /**< Base C type data.*/
    OX_Value  vty; /**< Pointed value's type.*/
    ssize_t   len; /**< Number of items in the array.*/
} OX_CPtrType;

/**
 * C array type.
 * Array type is used in field of structure or variable.
 * If parameter's type is an array, use pointer as its type.
 */
typedef struct {
    OX_CType  cty; /**< Base C type data.*/
    OX_Value  ity; /**< Item's type.*/
    ssize_t   len; /**< Number of items in the array.*/
} OX_CArrayType;

/** Field of the C structure.*/
typedef struct {
    OX_List      ln;     /**< List node data.*/
    OX_HashEntry he;     /**< Hash table entry.*/
    OX_Value     cty;    /**< Type of the field.*/
    size_t       offset; /**< Offset of the field.*/
} OX_CField;

/** C structure type.*/
typedef struct {
    OX_CType  cty;    /**< Base C type data.*/
    OX_List   f_list; /**< Fields list.*/
    OX_Hash   f_hash; /**< Fields hash table.*/
} OX_CStructType;

/** C array's information.*/
typedef struct {
    ssize_t  len;   /**< Array's length.*/
    ssize_t  isize; /**< Item's size.*/
} OX_CArrayInfo;

/** C value buffer.*/
typedef union {
    int8_t   i8;   /**< 8 bits integer.*/
    int16_t  i16;  /**< 16 bits integer.*/
    int32_t  i32;  /**< 32 bits integer.*/
    int64_t  i64;  /**< 64 bits integer.*/
    uint8_t  u8;   /**< 8 bits unsigned integer.*/
    uint16_t u16;  /**< 16 bits unsigned integer.*/
    uint32_t u32;  /**< 32 bits unsigned integer.*/
    uint64_t u64;  /**< 64 bits unsigned integer.*/
    float    f32;  /**< 32 bits float point number.*/
    double   f64;  /**< 64 bits float point number.*/
    void    *p;    /**< Pointer.*/
} OX_CVBuf;

/** C pointer owned type.*/
typedef enum {
    OX_CPTR_NON_OWNER = (0),      /**< The value is not the owner.*/
    OX_CPTR_INTERNAL  = (1 << 0), /**< The value is the owner, and the buffer is managed by OX.*/
    OX_CPTR_EXTERNAL  = (1 << 1), /**< The value is the owner, and the buffer is not managed by OX.*/
    OX_CPTR_ALLOC     = (1 << 2)  /**< Allocate a new C value.*/
} OX_CPtrOwned;

/** C value's parameters.*/
typedef struct {
    OX_CVBuf      v;     /**< The value buffer.*/
    OX_Value     *base;  /**< Base object of the C value.*/
    OX_CPtrOwned  own;   /**< Owner type.*/
    size_t        size;  /**< The size of the buffer.*/
} OX_CValueInfo;

/** C function's type.*/
typedef struct {
    OX_CType   cty;    /**< Base C type data.*/
    OX_Value   rty;    /**< Return value's type.*/
    OX_Value  *atys;   /**< Arguments' types.*/
    size_t     argc;   /**< Arguments' count.*/
    ffi_cif    cif;    /**< CIF data of FFI.*/
    ffi_type **atypes; /**< Arguments' FFI types.*/
    OX_Bool    vaarg;  /**< The function has vriable arguments.*/
} OX_CFuncType;

/** C value.*/
typedef struct {
    OX_Object o;     /**< Base object data.*/
    OX_Value  cty;   /**< C type of the value.*/
    OX_Value  base;  /**< The base object contains this value.*/
    OX_CVBuf  v;     /**< Value data.*/
} OX_CValue;

/** C function pointer.*/
typedef struct {
    OX_Value     cty;     /**< The C type of the C function pointer.*/
    OX_Value     v;       /**< The value of the C function pointer.*/
    void        *p;       /**< The function's pointer.*/
    ffi_closure *closure; /**< FFI closure.*/
    OX_VM       *vm;      /**< The virtual machine.*/
} OX_CFuncPtr;

/** Base virtual machine.*/
typedef struct {
    OX_Value v_null; /**< null value.*/;
} OX_BaseVM;

/** Base context data.*/
typedef struct {
    OX_VM          *vm;         /**< The virtual machine contains this context.*/
    OX_ValueBuffer *v_stack;    /**< Value stack.*/
    int             sched_cnt;  /**< Schedule counter.*/
    void           *priv;       /**< Private data.*/
} OX_BaseContext;

/** Get pointer of the null value.*/
#define ox_value_null(c) (&((OX_BaseVM*)((OX_BaseContext*)(c))->vm)->v_null)

/**
 * Assert the value must not null.
 * The program will be terminated if the valye is null.
 * @param p The value.
 * @return The value p.
 */
static inline const void*
ox_not_null (const void *p)
{
    assert(p);
    return p;
}

/**
 * Assert the value is not error.
 * The program will be terminated if the valye is OX_ERR.
 * @param r The result value.
 * @return the result value r.
 */
static inline OX_Result
ox_not_error (OX_Result r)
{
    assert(r != OX_ERR);
    return r;
}

#ifdef __cplusplus
}
#endif

#endif
