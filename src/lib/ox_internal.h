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
 * OX interanl header.
 */

#ifndef _OX_INTERNAL_H_
#define _OX_INTERNAL_H_

#define _GNU_SOURCE
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <iconv.h>
#include <limits.h>
#include <libgen.h>
#include <errno.h>
#include <fcntl.h>
#include <libintl.h>
#include <locale.h>

#ifdef ARCH_LINUX
#include <endian.h>
#include <dlfcn.h>
#include <sys/mman.h>
#define OX_SUPPORT_PTHREAD
#define OX_SUPPORT_MMAP
#define OX_SUPPORT_COLOR_TTY
#endif /*ARCH_LINUX*/

#ifdef ARCH_WIN
#include <windows.h>
#include <shlwapi.h>
#define OX_SUPPORT_COLOR_TTY

/*#define OX_SUPPORT_PTHREAD*/
extern int   dlclose(void *handle);
extern void* dlsym(void *handle, const char *symbol);
extern char* realpath(const char *path, char *resolved_path);
#ifndef OX_SUPPORT_PTHREAD
typedef HANDLE             OX_Thread;
typedef DWORD              OX_ThreadKey;
typedef CRITICAL_SECTION   OX_Mutex;
typedef CONDITION_VARIABLE OX_CondVar;
#endif
#endif /*ARCH_WIN*/

#ifdef OX_SUPPORT_PTHREAD
#include <pthread.h>
typedef pthread_t       OX_Thread;
typedef pthread_key_t   OX_ThreadKey;
typedef pthread_mutex_t OX_Mutex;
typedef pthread_cond_t  OX_CondVar;
#endif

#define OX_LOG_LEVEL OX_LOG_LEVEL_WARNING
#include <ox.h>

#ifdef textdomain
#undef textdomain
#endif

#include "ox_keyword.h"
#include "ox_string_id.h"
#include "ox_object_id.h"
#include "ox_ast.h"
#include "ox_bytecode.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Matched slice.*/
typedef struct {
    ssize_t start; /**< Start position.*/
    ssize_t end;   /**< End position.*/
} OX_Slice;

/** Match result.*/
typedef struct {
    OX_Value  s;            /**< The original string.*/
    size_t    start;        /**< Match start position.*/
    size_t    end;          /**< Match end position.*/
    OX_Value  sub;          /**< Matched substrings.*/
    OX_Value  group_strs;   /**< Substrings array of the matched group.*/
    OX_Value  group_slices; /**< Slices array of the matched group.*/
    size_t    group_num;    /**< Number of groups.*/
    OX_Slice *slices;       /**< Slices array.*/
} OX_Match;

/** String input.*/
typedef struct {
    OX_Input  input; /**< Base input data.*/
    OX_Value  s;     /**< The string value.*/
    size_t    pos;   /**< Current read position.*/
} OX_StringInput;

/** String output format type.*/
typedef enum {
    OX_SOUT_FMT_STR,   /**< Basic string.*/
    OX_SOUT_FMT_DEC,   /**< Decimal integer number.*/
    OX_SOUT_FMT_UDEC,  /**< Unsigned decimal integer number.*/
    OX_SOUT_FMT_OCT,   /**< Octal integer number.*/
    OX_SOUT_FMT_HEX,   /**< Hexadecimal integer number.*/
    OX_SOUT_FMT_FLOAT, /**< Float point number.*/
    OX_SOUT_FMT_EXP,   /**< Exponent mode float point number.*/
    OX_SOUT_FMT_NUMBER,/**< Number.*/
    OX_SOUT_FMT_CHAR   /**< Character.*/
} OX_SoutFormat;

/** Align to the head of the string.*/
#define OX_SOUT_FL_ALIGN_HEAD (1 << 24)
/** Add 0 before the number.*/
#define OX_SOUT_FL_ZERO       (1 << 25)

/** Get the string's format.*/
#define OX_SOUT_GET_FMT(v)    ((v) & 0xff)
/** Get the string's width.*/
#define OX_SOUT_GET_WIDTH(v)  (((v) >> 8) & 0xff)
/** Get the number's output precision.*/
#define OX_SOUT_GET_PREC(v)   (((v) >> 16) & 0xff)

/** Default string output width.*/
#define OX_SOUT_WIDTH_DEFAULT 0xff
/** Default number precision.*/
#define OX_SOUT_PREC_DEFAULT  0xff

/** Make the string output flags.*/
#define OX_SOUT_FLAGS_MAKE(f, w, p, m)\
    ((f) | ((w) << 8) | ((p) << 16) | (m))

/** Token type.*/
typedef enum {
    OX_TOKEN_END  = -1,  /**< The input is end.*/
    OX_TOKEN_NULL = 256, /**< null.*/
    OX_TOKEN_BOOL,       /**< Boolean literal.*/
    OX_TOKEN_NUMBER,     /**< Number literal.*/
    OX_TOKEN_STRING,     /**< String literal.*/
    OX_TOKEN_STR_HEAD,   /**< String's head part.*/
    OX_TOKEN_STR_MID,    /**< String's middle part.*/
    OX_TOKEN_STR_TAIL,   /**< String's tail part.*/
    OX_TOKEN_RE,         /**< Regular expression.*/
    OX_TOKEN_ID,         /**< Identifier.*/
    OX_TOKEN_AT_ID,      /**< @Identifier.*/
    OX_TOKEN_HASH_ID,    /**< #Identifier.*/
    OX_TOKEN_DOC,        /**< Document string.*/
#include "ox_punct.h"
} OX_TokenType;

/** Token of the source file.*/
typedef struct {
    int          type;    /**< Type of the token.*/
    OX_Keyword   keyword; /**< The keyword type if the token is an identifier.*/
    OX_Location  loc;     /**< Location of the token.*/
    OX_Value    *v;       /**< The value of the token.*/
} OX_Token;

/** Get string format from the lexical analyzer.*/
#define OX_LEX_FL_STR_FMT (1 << 0)
/** "/" or "/=" can be here.*/
#define OX_LEX_FL_DIV     (1 << 1)
/** No embedded expression in string.*/
#define OX_LEX_FL_NO_EMBED_EXPR (1 << 2)

/** The lexical analyzer has error.*/
#define OX_LEX_ST_ERR       (1 << 0)
/** The lexical analyzer is analyzing the script body.*/
#define OX_LEX_ST_BODY      (1 << 1)
/** The lexical analyzer do not show prompt message.*/
#define OX_LEX_ST_NO_PROMPT (1 << 2)
/** Analyze document data.*/
#define OX_LEX_ST_DOC       (1 << 3)

/** The lexical analyzer's current string type.*/
typedef enum {
    OX_LEX_STR_DOUBLE_QUOT, /**< "..."*/
    OX_LEX_STR_SINGLE_QUOT  /**< ''...'' */
} OX_LexStrType;

/** The lexical analyzer's current string state.*/
typedef struct {
    OX_LexStrType type;  /**< The string's type.*/
    int           brace; /**< The brace level.*/
} OX_LexStrState;

/** OX lexical analyzer.*/
typedef struct {
    int            status; /**< Status of the lexical analyzer.*/
    OX_Input      *input;  /**< The input of the source.*/
    OX_CharBuffer  text;   /**< Text buffer.*/
    int            brace_level; /**< Brace level.*/
    OX_VECTOR_TYPE_DECL(OX_LexStrState) str_stack; /**< Brace level stack.*/
    OX_Value      *doc;    /**< The document object.*/
} OX_Lex;

/** The byte code script.*/
typedef struct OX_BcScript_s OX_BcScript;

/** Block's content type.*/
typedef enum {
    OX_BLOCK_CONTENT_STMT, /**< Statement.*/
    OX_BLOCK_CONTENT_ITEM, /**< Array item.*/
    OX_BLOCK_CONTENT_PROP  /**< Object's property.*/
} OX_BlockContentType;

/*Declaration type.*/
typedef enum {
    OX_DECL_CONST, /**< Define a constant.*/
    OX_DECL_PARAM, /**< Define a parameter.*/
    OX_DECL_VAR,   /**< Define a variable.*/
    OX_DECL_REF,   /**< Reference to other script.*/
    OX_DECL_OUTER  /**< Reference to an outer identifier.*/
} OX_DeclType;

/** Declaration type mask.*/
#define OX_DECL_TYPE_MASK  0xff
/** Declaration is an auto closed variable.*/
#define OX_DECL_AUTO_CLOSE (1 << 8) 

/** Script declaration entry.*/
typedef struct {
    OX_HashEntry he;   /**< Hash table entry.*/
    OX_List      ln;   /**< List node data.*/
    uint16_t     type; /**< Declaration type.*/
    uint16_t     id;   /**< The value index in the frame.*/
} OX_ScriptDecl;

/** The function has this argument.*/
#define OX_SCRIPT_FUNC_FL_THIS (1 << 0)

/** Script function data.*/
typedef struct {
    OX_BcScript *script;    /**< The byte code script contains this function.*/
    OX_Hash      decl_hash; /**< Declaration hash table.*/
    OX_List      decl_list; /**< Declaration list.*/
    size_t       loc_start; /**< Location offset.*/
    size_t       bc_start;  /**< Byte code start offset.*/
    uint16_t     loc_len;   /**< Location length.*/
    uint16_t     bc_len;    /**< Byte code length of the function.*/
    uint8_t      frame_num; /**< Referenced frames' number.*/
    uint8_t      reg_num;   /**< Used regisers' number.*/
    uint16_t     flags;     /**< Flags.*/
} OX_ScriptFunc;

/** Script reference item.*/
typedef struct {
    OX_Value orig; /**< Origin name of the item.*/
    OX_Value name; /**< The new local name of the item.*/
} OX_ScriptRefItem;

/** Script reference.*/
typedef struct {
    OX_Value filename;   /**< The filename of the scipt.*/
    OX_Value script;     /**< The referenced script.*/
    size_t   item_start; /**< The first item's index.*/
    size_t   item_num;   /**< Number of items.*/
} OX_ScriptRef;

/** Value frame.*/
typedef struct OX_Frame_s OX_Frame;
/** Value frame.*/
struct OX_Frame_s {
    OX_GcObject  gco;  /**< Base GC managed object data.*/
    OX_Value     func; /**< The function.*/
    int          ip;   /**< Insruction pointer.*/
    size_t       len;  /**< Length of the value buffer.*/
    OX_Frame    *bot;  /**< The bottom frame.*/
    OX_Value     thiz; /**< This argument.*/
    OX_Value    *v;    /**< The value buffer.*/
};

/** The script's running state.*/
typedef enum {
    OX_SCRIPT_STATE_ERROR,  /**< Error.*/
    OX_SCRIPT_STATE_UNINIT, /**< Uninitialized.*/
    OX_SCRIPT_STATE_INITED, /**< Initialized.*/
    OX_SCRIPT_STATE_CALLED  /**< Called.*/
} OX_ScriptState;

/** Script public entry.*/
typedef struct {
    OX_HashEntry he; /**< Hash table entry.*/
    OX_List      ln; /**< List node data.*/
    size_t       id; /**< The value index in the frame.*/
} OX_ScriptPublic;

/** Location of the script.*/
typedef struct {
    uint16_t line; /**< The line nunmber.*/
    uint16_t ip;   /**< Instruction pointer.*/
} OX_ScriptLoc;

/** Script.*/
typedef struct OX_Script_s OX_Script;

/** Script*/
struct OX_Script_s {
    OX_GcObject       gco;          /**< Base GC managed object data.*/
    OX_ScriptState    state;        /**< State of the script.*/
    OX_Value          error;        /**< The error value.*/
    OX_HashEntry      he;           /**< Hash table entry data.*/
    OX_Hash           pub_hash;     /**< The public declarations hash table.*/
    OX_List           pub_list;     /**< The public declarations list.*/
    OX_Value          func;         /**< The function value of the script.*/
    OX_Frame         *frame;        /**< The bottom value frame.*/
    OX_ScriptRef     *refs;         /**< Reference array.*/
    size_t            ref_num;      /**< Number of reference.*/
    OX_ScriptRefItem *ref_items;    /**< Reference items array.*/
    size_t            ref_item_num; /**< Number of reference items.*/
    OX_Value          text_domain;  /**< Text domain.*/
};

/** Bytecode script.*/
struct OX_BcScript_s {
    OX_Script         script;       /**< Base script data.*/
    OX_BcScript      *base;         /**< Is the script is generated by "eval" or "Function", pointed to the script create it.*/
    OX_Input         *input;        /**< The input of the script.*/
    OX_Value         *cvs;          /**< Constant value array.*/
    size_t            cv_num;       /**< Number of contant values.*/
    OX_Value         *pps;          /**< Private property array.*/
    size_t            pp_num;       /**< Number of private properties.*/
    OX_Value         *ts;           /**< Text string array.*/
    OX_Value         *lts;          /**< Localized text string array.*/
    size_t            t_num;        /**< Number of localized text strings.*/
    OX_Value         *tts;          /**< Text string template array.*/
    OX_Value         *ltts;         /**< Localized text string template array.*/
    size_t            tt_num;       /**< Number of localized string templates.*/
    OX_ScriptFunc    *sfuncs;       /**< Script functions array.*/
    size_t            sfunc_num;    /**< Number of script functions in the script.*/
    uint8_t          *bc;           /**< Bytecode buffer.*/
    size_t            bc_len;       /**< Length of the bytecode buffer.*/
    OX_ScriptLoc     *loc_tab;      /**< Location table.*/
    size_t            loc_tab_len;  /**< Length of the location table.*/
};

/** Native script.*/
typedef struct {
    OX_Script  script;     /**< Base script data.*/
    void      *handle;     /**< Library handle.*/
    size_t     frame_size; /**< Size of the frame.*/
} OX_NativeScript;

/** Function.*/
typedef struct {
    OX_Object       o;      /**< Base object data.*/
    OX_ScriptFunc  *sfunc;  /**< The script function of this function.*/
    OX_Frame      **frames; /**< The referenced frames.*/
} OX_Function;

/** Native function.*/
typedef struct {
    OX_Object  o;      /**< Base object data.*/
    OX_Script *script; /**< The script contains this function.*/
    OX_CFunc   cf;     /**< C function's pointer.*/
} OX_NativeFunc;

/** Stack entry's type.*/
typedef enum {
    OX_STACK_STR,     /**< Multipart string.*/
    OX_STACK_CALL,    /**< Call.*/
    OX_STACK_TRY,     /**< Try/catch.*/
    OX_STACK_ITER,    /**< Iterator.*/
    OX_STACK_APAT,    /**< Array pattern.*/
    OX_STACK_OPAT,    /**< Object pattern.*/
    OX_STACK_PARAM,   /**< Parameters.*/
    OX_STACK_ARRAY,   /**< Array.*/
    OX_STACK_OBJECT,  /**< Object.*/
    OX_STACK_ENUM,    /**< Enumeration.*/
    OX_STACK_BITFIELD,/**< Bitfields*/
    OX_STACK_SCHED,   /**< Schedule.*/
    OX_STACK_RETURN   /**< Return.*/
} OX_StackType;

/** Try state.*/
typedef enum {
    OX_TRY_STATE_TRY,     /**< In try block.*/
    OX_TRY_STATE_CATCH,   /**< In catch block.*/
    OX_TRY_STATE_FINALLY  /**< In finally block.*/
} OX_TryState;

/** Value entry.*/
typedef struct {
    OX_HashEntry he; /**< Hash table entry.*/
    OX_Value     k;  /**< Key of the entry.*/
    OX_Value     v;  /**< Value of the entry.*/
} OX_ValueEntry;

/** Run status record.*/
typedef struct {
    OX_Value    *args; /**< Arguments.*/
    size_t       argc; /**< Arguments' count.*/
    OX_Value    *rv;   /**< Return value.*/
    OX_Frame    *frame;/**< The frame pointer.*/
    size_t       vp;   /**< Value stack pointer.*/
    size_t       sp;   /**< Status stack pointer.*/
} OX_RunStatusRec;

/** Stack entry.*/
typedef struct {
    OX_StackType type; /**< Entry's type.*/
    union {
        struct {
            OX_Value *f;      /**< Function.*/
            OX_Value *args;   /**< Arguments.*/
            size_t    argc;   /**< Arguments' count.*/
        } s;                  /**< String entry data.*/
        struct {
            OX_Value *f;      /**< Function.*/
            OX_Value *thiz;   /**< This argument.*/
            OX_Value *args;   /**< Arguments.*/
            OX_Value *iter;   /**< Iterator.*/
            size_t    argc;   /**< Arguments' count.*/
        } c;                  /**< Call entry data.*/
        struct {
            OX_TryState state;         /**< The current state.*/
            OX_Result   r;             /**< Result.*/
            int         catch_label;   /**< Catch jump label.*/
            int         finally_label; /**< Finally jump label.*/
            size_t      jmp_ip;        /**< Deep jump's instruction pointer.*/
            size_t      jmp_sp;        /**< Deep jump's stack pointer.*/
        } t;                           /**< Try entry data.*/
        struct {
            OX_Value *iter;   /**< Iterator.*/
        } i;                  /**< Iterator entry data.*/
        struct {
            OX_Value *a;      /**< Array.*/
            size_t    id;     /**< Current item's index.*/
        } a;                  /**< Array pattern entry data.*/
        struct {
            OX_Value *o;      /**< Object.*/
            OX_Hash   p_hash; /**< Solved properties hash table.*/
        } o;                  /**< Object pattern entry data.*/
        struct {
            size_t    id;     /**< Index of parameter.*/
        } p;                  /**< Parameters entry data.*/
        struct {
            OX_Value *c;      /**< The container object.*/
            OX_Value *e;      /**< The enumeration object.*/
            int       v;      /**< The current item's value.*/
        } e;                  /**< Enumeration entry data.*/
        struct {
            OX_Value *c;      /**< The container object.*/
            OX_Value *b;      /**< The bitfield object.*/
            int       v;      /**< The current item's value.*/
        } b;                  /**< Bitfield entry data.*/
        OX_RunStatusRec r;    /**< Run status record.*/
    } s;                      /**< Stack entry's data.*/
} OX_Stack;

/** Package lookup directory.*/
typedef struct {
    OX_List  ln;  /**< List node data.*/
    char    *dir; /**< Directory.*/
} OX_PackageDir;

/** Global reference entry.*/
typedef struct {
    OX_HashEntry he;  /**< Hash table entry.*/
    int          ref; /**< Reference counter.*/
} OX_GlobalRef;

/** Status buffer.*/
typedef OX_VECTOR_TYPE_DECL(OX_Stack) OX_StatusBuffer;

/** State of the fiber.*/
typedef enum {
    OX_FIBER_STATE_INIT, /**< Initialized.*/
    OX_FIBER_STATE_RUN,  /**< Running.*/
    OX_FIBER_STATE_END,  /**< End.*/
    OX_FIBER_STATE_ERROR /**< Error.*/
} OX_FiberState;

/** Fiber.*/
typedef struct {
    OX_FiberState   state;   /**< State of the fiber.*/
    OX_Value        func;    /**< The function of the fiber.*/
    OX_Value        rv;      /**< The return value of the fiber.*/
    OX_Value       *args;    /**< The arguments of the fiber.*/
    size_t          argc;    /**< The arguments' count.*/
    OX_Value       *yr;      /**< The value pointer to store the yield result.*/
    OX_StatusBuffer s_stack; /**< Status stack.*/
    OX_ValueBuffer  v_stack; /**< Value stack.*/
    OX_RunStatusRec rsr;     /**< Run status record.*/
} OX_Fiber;

/** Virtual machine.*/
struct OX_VM_s {
    OX_BaseVM       base;         /**< The base virtual machine.*/
    OX_LogLevel     log_level;    /**< Log output level.*/
    OX_LogField     log_fields;   /**< Log output fields.*/
    FILE           *log_file;     /**< Log output file.*/
    size_t          mem_allocted; /**< Allocated memory size.*/
    size_t          mem_max_allocated; /**< Maximum allocate memory size.*/
    size_t          gc_start_size;/**< The memory size start running garbage collecter.*/
    size_t          gc_last_size; /**< The memory size after last garbage collection.*/
    OX_Bool         gc_marked_full;    /**< Marked GC object stack full flag.*/
    int             gc_scan_cnt;  /**< GC scanning counter.*/
    OX_VECTOR_TYPE_DECL(OX_GcObject*) gc_marked_stack; /**< Marked GC object stack.*/
    OX_GcObject    *gco_list;     /**< The GC managed objects.*/
    OX_Hash         str_singleton_hash;/**< Singleton string hash table.*/
    char           *file_enc;     /**< File's character encoding.*/
    char           *install_dir;  /**< OX installation directory.*/
    OX_Bool         dump_throw;   /**< Dump stack when throw an error.*/
    OX_Value        strings[OX_STR_ID_MAX]; /**< Strings table.*/
    OX_Value        objects[OX_OBJ_ID_MAX]; /**< Objects table.*/
    OX_Hash         script_hash;  /**< Script hash table.*/
    OX_List         pkg_dirs;     /**< Package directories list.*/
    OX_Value        packages;     /**< The package information map.*/
    OX_Mutex        lock;         /**< The lock of the virtual machine.*/
    OX_ThreadKey    key;          /**< Thread key of the context.*/
    OX_List         ctxt_list;    /**< Contexts list.*/
    int             ref;          /**< Reference counter.*/
    OX_Hash         global_ref_hash; /**< Global reference hash table.*/
};

/** Running context.*/
struct OX_Context_s {
    OX_BaseContext   base;         /**< The base context data.*/
    OX_StatusBuffer *s_stack;      /**< The current status stack.*/
    OX_ValueBuffer   bot_v_stack;  /**< The bottom value stack.*/
    OX_StatusBuffer  bot_s_stack;  /**< The bottom status stack.*/
    OX_List          ln;           /**< List node data.*/
    OX_Frame        *frames;       /**< The value frame stack.*/
    OX_Frame        *error_frames; /**< Error frame stack.*/
    OX_Frame        *main_frames;  /**< The main frame stack.*/
    OX_Value         error;        /**< The error value.*/
    OX_Script       *curr_script;  /**< The current script.*/
    int              lock_cnt;     /**< Lock counter.*/
};

/** Get the internal string value.*/
#define OX_STRING(c, s) (&(c)->base.vm->strings[OX_STR_ID_##s])
/** Get the internal object value.*/
#define OX_OBJECT(c, o) (&(c)->base.vm->objects[OX_OBJ_ID_##o])

/**
 * Show the input's text of the location.
 * @param ctxt The current running context.
 * @param input The input.
 * @param loc Location of the text.
 * @param col Color of the text.
 */
extern void
ox_input_show_text (OX_Context *ctxt, OX_Input *input, OX_Location *loc, const char *col);

/**
 * Initialize the log data in the running context.
 * @param ctxt The running context.
 */
extern void
ox_log_init (OX_Context *ctxt);

/**
 * Release the log data in the running context.
 * @param ctxt The running context.
 */
extern void
ox_log_deinit (OX_Context *ctxt);

/**
 * Initialize the memory manager data in the context.
 * @param ctxt The running context.
 */
extern void
ox_mem_init (OX_Context *ctxt);

/**
 * Release the memry manager data in the context.
 * @param ctxt The running context.
 */
extern void
ox_mem_deinit (OX_Context *ctxt);

/**
 * Initialize the garbage collecter.
 * @param ctxt The running context.
 */
extern void
ox_gc_init (OX_Context *ctxt);

/**
 * Release the garbage collecter.
 * @param ctxt The running context.
 */
extern void
ox_gc_deinit (OX_Context *ctxt);

/**
 * Initialize the singleton string hash table.
 * @param ctxt The running context.
 */
extern void
ox_string_singleton_init (OX_Context *ctxt);

/**
 * Release the singleton string hash table.
 * @param ctxt The running context.
 */
extern void
ox_string_singleton_deinit (OX_Context *ctxt);

/**
 * Initialize the lexical analyzer.
 * @param ctxt The current running context.
 * @param lex The lexical analyzer to be initialized.
 * @param input The character input.
 */
extern void
ox_lex_init (OX_Context *ctxt, OX_Lex *lex, OX_Input *input);

/**
 * Release the lexical analyzer.
 * @param ctxt The current running context.
 * @param lex The lexical analyzer to be released.
 */
extern void
ox_lex_deinit (OX_Context *ctxt, OX_Lex *lex);

/**
 * Get a token from the lexical analyzer.
 * @param ctxt The current running context.
 * @param lex The lexical analyzer.
 * @param flags Analyse flags.
 * @param[out] tok Return the token.
 */
extern void
ox_lex_token (OX_Context *ctxt, OX_Lex *lex, OX_Token *tok, int flags);

/**
 * Get the token type's name.
 * @param ctxt The current running context.
 * @param cb The output character buffer to store the name.
 * @param type The token type or keyword type. 
 */
extern void
ox_token_type_get_name (OX_Context *ctxt, OX_CharBuffer *cb, int type);

/**
 * Initialize the boolean class.
 * @param ctxt The current running context.
 */
extern void
ox_bool_class_init (OX_Context *ctxt);

/**
 * Initialize the number class.
 * @param ctxt The current running context.
 */
extern void
ox_number_class_init (OX_Context *ctxt);

/**
 * Initialize the string class.
 * @param ctxt The current running context.
 */
extern void
ox_string_class_init (OX_Context *ctxt);

/**
 * Initialize the array class.
 * @param ctxt The current running context.
 */
extern void
ox_array_class_init (OX_Context *ctxt);

/**
 * Initialize the function class.
 * @param ctxt The current running context.
 */
extern void
ox_func_class_init (OX_Context *ctxt);

/**
 * Initialize the iterator class.
 * @param ctxt The current running context.
 */
extern void
ox_iterator_class_init (OX_Context *ctxt);

/**
 * Initialize the object class.
 * @param ctxt The current running context.
 */
extern void
ox_object_class_init (OX_Context *ctxt);

/**
 * Initialize the Error object.
 * @param ctxt The current running context.
 */
extern void
ox_error_class_init (OX_Context *ctxt);

/**
 * Initialize the script object.
 * @param ctxt The current running context.
 */
extern void
ox_script_object_init (OX_Context *ctxt);

/**
 * Initialize the enumeration object.
 * @param ctxt The current running context.
 */
extern void
ox_enum_object_init (OX_Context *ctxt);

/**
 * Initialize the regular expression class.
 * @param ctxt The current running context.
 */
extern void
ox_re_class_init (OX_Context *ctxt);

/**
 * Initialize the match class.
 * @param ctxt The current running context.
 */
extern void
ox_match_class_init (OX_Context *ctxt);

/**
 * Initialize the set class.
 * @param ctxt The current running context.
 */
extern void
ox_set_class_init (OX_Context *ctxt);

/**
 * Initialize the dictionary class.
 * @param ctxt The current running context.
 */
extern void
ox_dict_class_init (OX_Context *ctxt);

/**
 * Initialize the C type classes.
 * @param ctxt The current running context.
 */
extern void
ox_ctype_class_init (OX_Context *ctxt);

/**
 * Initialize the proxy classes.
 * @param ctxt The current running context.
 */
extern void
ox_proxy_class_init (OX_Context *ctxt);

/**
 * Initialize the AST classes.
 * @param ctxt The current running context.
 */
extern void
ox_ast_class_init (OX_Context *ctxt);

/**
 * Add AST enumerations.
 * @param ctxt The current running context.
 */
extern void
ox_ast_add_enums (OX_Context *ctxt);

/**
 * Create a new AST node.
 * @param ctxt The current running context.
 * @param[out] ast Return the new AST node.
 * @param type The node's type.
 */
extern void
ox_ast_new (OX_Context *ctxt, OX_Value *ast, OX_AstType type);

/**
 * Get the AST node's type.
 * @param ctxt The current running context.
 * @param ast The AST node.
 * @return The type of the node.
 */
extern OX_AstType
ox_ast_get_type (OX_Context *ctxt, OX_Value *ast);

/**
 * Get the AST node's location.
 * @param ctxt The current running context.
 * @param ast The AST node.
 * @param[out] loc Return the location of the node.
 */
extern void
ox_ast_get_loc (OX_Context *ctxt, OX_Value *ast, OX_Location *loc);

/**
 * Set the AST node's location.
 * @param ctxt The current running context.
 * @param ast The AST node.
 * @param loc The location of the node.
 */
extern void
ox_ast_set_loc (OX_Context *ctxt, OX_Value *ast, OX_Location *loc);

/**
 * Push a new frame to the stack.
 * @param ctxt The current running context.
 * @param func The function use this frame.
 * @param len Frame's value buffer length.
 * @return The new frame.
 */
extern OX_Frame*
ox_frame_push (OX_Context *ctxt, OX_Value *func, size_t len);

/**
 * Popup the top frame from the stack.
 * @param ctxt The current running context.
 */
static inline void
ox_frame_pop (OX_Context *ctxt)
{
    OX_Frame *f = ctxt->frames;

    assert(f);

    ctxt->frames = f->bot;
}

/**
 * Get the top ox frame from the current stack.
 * @param ctxt The current running context.
 * @return The top OX frame.
 */
static inline OX_Frame*
ox_frame_get (OX_Context *ctxt)
{
    OX_Frame *f = ctxt->frames;

    while (f && (ox_value_get_gco_type(ctxt, &f->func) != OX_GCO_FUNCTION))
        f = f->bot;

    return f;
}

/**
 * Create a new function.
 * @param ctxt The current running context.
 * @param[out] f Return the new function.
 * @param sfunc The script function use this frame.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_function_new (OX_Context *ctxt, OX_Value *f, OX_ScriptFunc *sfunc);

/**
 * Lookup the function's source code's line number.
 * @param ctxt The current running context.
 * @param f The function.
 * @param ip The instruction pointer.
 * @return The line number.
 * @retval -1 Cannot find the line.
 */
extern int
ox_function_lookup_line (OX_Context *ctxt, OX_Value *f, int ip);

/**
 * Initialize the script hash table.
 * @param ctxt The current running context.
 */
extern void
ox_script_hash_init (OX_Context *ctxt);

/**
 * Release the script hash table.
 * @param ctxt The current running context.
 */
extern void
ox_script_hash_deinit (OX_Context *ctxt);

/**
 * Scan the scripts in the hash table.
 * @param ctxt The current running context.
 */
extern void
ox_gc_scan_script_hash (OX_Context *ctxt);

/**
 * Check if the value is a script.
 * @param ctxt The current running context.
 * @param v The value.
 * @retval OX_TRUE The value is a script.
 * @retval OX_FALSE The value is not a script.
 */
static inline OX_Bool
ox_value_is_script (OX_Context *ctxt, OX_Value *v)
{
    OX_GcObjectType type = ox_value_get_gco_type(ctxt, v);

    if (type == -1)
        return OX_FALSE;

    return (type & OX_GCO_FL_SCRIPT) ? OX_TRUE : OX_FALSE;
}

/**
 * Create a new script.
 * @param ctxt The current running context.
 * @param[out] sv Return the new script value.
 * @param path The path of the script.
 * @return The new script.
 * @retval NULL On error.
 */
extern OX_Script*
ox_script_new (OX_Context *ctxt, OX_Value *sv, OX_Value *path);

/**
 * Create a new byte code script.
 * @param ctxt The current running context.
 * @param[out] sv Return the new script value.
 * @param input The input of the script.
 * @param reg Register the script the manager or not.
 * @return The new script.
 * @retval NULL On error.
 */
extern OX_BcScript*
ox_bc_script_new (OX_Context *ctxt, OX_Value *sv, OX_Input *input, OX_Bool reg);

/**
 * Set the byte code script's base script to the caller.
 * @param ctxt The current running context.
 * @param script The script to be set.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_bc_script_set_base (OX_Context *ctxt, OX_Value *script);

/**
 * Allocate reference buffer for the script.
 * @param ctxt The current running context.
 * @param script The script.
 * @param ref_num Number of referenced files.
 * @param item_num Number of refereced items.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_script_alloc_refs (OX_Context *ctxt, OX_Script *script, size_t ref_num, size_t item_num);

/**
 * Add a declaration to the script function.
 * @param ctxt The current running context.
 * @param sfunc The script function.
 * @param type The declaration type.
 * @param name The name of the declaration.
 * @return The declaration's index in the frame.
 * @retval -1 On error.
 */
extern int
ox_script_func_add_decl (OX_Context *ctxt, OX_ScriptFunc *sfunc, OX_DeclType type, OX_Value *name);

/**
 * Lookup the source code's line number.
 * @param ctxt The current running context.
 * @param sf The script function.
 * @param ip The instruction pointer.
 * @return The line number.
 * @retval -1 Cannot find the line.
 */
extern int
ox_script_func_lookup_line (OX_Context *ctxt, OX_ScriptFunc *sf, int ip);

/**
 * Add a public declaration entry to the script.
 * @param ctxt The current running context.
 * @param script The script.
 * @param name The name of the declaration.
 * @param id The declaration's index in the frame.
 * If id == -1, allocate an index and return it.
 * @return The declaration's index in the frame.
 * @retval -1 On error.
 */
extern int
ox_script_add_public (OX_Context *ctxt, OX_Script *script, OX_Value *name, int id);

/**
 * Initialize the script.
 * @param ctxt The current running context.
 * @param script The script.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_script_init (OX_Context *ctxt, OX_Value *script);

/**
 * Call the function.
 * @param ctxt The current running context.
 * @param f The function value.
 * @param thiz This argument.
 * @param args Arguments.
 * @param argc Count of arguments.
 * @param[out] rv Return value.
 * @param fiber The current fiber.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_function_call (OX_Context *ctxt, OX_Value *f, OX_Value *thiz,
        OX_Value *args, size_t argc, OX_Value *rv, OX_Fiber *fiber);

/**
 * Call the native function.
 * @param ctxt The current running context.
 * @param f The native function value.
 * @param thiz This argument.
 * @param args Arguments.
 * @param argc Count of arguments.
 * @param[out] rv Return value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_native_func_call (OX_Context *ctxt, OX_Value *f, OX_Value *thiz,
        OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_NativeFunc *nf = ox_value_get_gco(ctxt, f);

    return nf->cf(ctxt, f, thiz, args, argc, rv);
}

/**
 * Get the top entry in the stack.
 * @param ctxt The current running context.
 * @return The top stack entry.
 */
static inline OX_Stack*
ox_stack_top (OX_Context *ctxt)
{
    return &ox_vector_item(ctxt->s_stack, ox_vector_length(ctxt->s_stack) - 1);
}

/**
 * Push a new entry to the stack.
 * @param ctxt The current running context.
 * @param type The entry's type.
 * @return The new stack entry.
 */
static inline OX_Stack*
ox_stack_push (OX_Context *ctxt, OX_StackType type)
{
    size_t i = ox_vector_length(ctxt->s_stack);
    OX_Stack *s;

    ox_not_error(ox_vector_expand(ctxt, ctxt->s_stack, i + 1));

    s = &ox_vector_item(ctxt->s_stack, i);
    s->type = type;

    return s;
}

/**
 * Release a sstack entry.
 * @param ctxt The current running context.
 * @param se The stack entry to be released.
 */
static inline void
ox_stack_deinit (OX_Context *ctxt, OX_Stack *se)
{
    switch (se->type) {
    case OX_STACK_OBJECT:
    case OX_STACK_OPAT: {
        OX_ValueEntry *e, *ne;
        size_t i;

        ox_hash_foreach_safe_c(&se->s.o.p_hash, i, e, ne, OX_ValueEntry, he) {
            OX_DEL(ctxt, e);
        }

        ox_hash_deinit(ctxt, &se->s.o.p_hash);
        break;
    }
    default:
        break;
    }
}

/**
 * Popup the top entry from the stack.
 * @param ctxt The current running context.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_stack_pop (OX_Context *ctxt)
{
    OX_Stack *se = ox_stack_top(ctxt);
    OX_Result r = OX_OK;

    switch (se->type) {
    case OX_STACK_STR:
        ox_value_stack_pop(ctxt, se->s.s.f);
        break;
    case OX_STACK_CALL:
        ox_value_stack_pop(ctxt, se->s.c.f);
        break;
    case OX_STACK_ITER:
        r = ox_close(ctxt, se->s.i.iter);
        ox_value_stack_pop(ctxt, se->s.i.iter);
        break;
    case OX_STACK_ARRAY:
    case OX_STACK_APAT:
        ox_value_stack_pop(ctxt, se->s.a.a);
        break;
    case OX_STACK_OBJECT:
    case OX_STACK_OPAT: {
        OX_ValueEntry *e, *ne;
        size_t i;

        ox_hash_foreach_safe_c(&se->s.o.p_hash, i, e, ne, OX_ValueEntry, he) {
            OX_DEL(ctxt, e);
        }

        ox_hash_deinit(ctxt, &se->s.o.p_hash);
        ox_value_stack_pop(ctxt, se->s.o.o);
        break;
    }
    case OX_STACK_ENUM:
        ox_value_stack_pop(ctxt, se->s.e.c);
        break;
    case OX_STACK_BITFIELD:
        ox_value_stack_pop(ctxt, se->s.b.c);
        break;
    case OX_STACK_SCHED:
        ctxt->base.sched_cnt --;
        break;
    case OX_STACK_RETURN:
        ox_frame_pop(ctxt);
        break;
    default:
        break;
    }

    ctxt->s_stack->len --;
    return r;
}

/**
 * Scan referenced objects in the stack.
 * @param ctxt The current running context.
 * @param c The context to be scanned.
 */
static inline void
ox_gc_scan_stack (OX_Context *ctxt, OX_StatusBuffer *sb)
{
    OX_Stack *s, *se;

    s = sb->items;
    se = s + sb->len;
    while (s < se) {
        switch (s->type) {
        case OX_STACK_PARAM: {
            OX_ValueEntry *e;
            size_t i;

            ox_hash_foreach_c(&s->s.o.p_hash, i, e, OX_ValueEntry, he) {
                ox_gc_scan_value(ctxt, &e->k);
                ox_gc_scan_value(ctxt, &e->v);
            }
            break;
        }
        default:
            break;
        }
        s ++;
    }
}

/**
 * Call the boolean class.
 * @param ctxt The current running context.
 * @param o The boolean class object.
 * @param thiz This argument.
 * @param args Arguments.
 * @param argc Arguments' count.
 * @param[out] rv Return value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_bool_class_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv);

/**
 * Call the number class.
 * @param ctxt The current running context.
 * @param o The number class object.
 * @param thiz This argument.
 * @param args Arguments.
 * @param argc Arguments' count.
 * @param[out] rv Return value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_number_class_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv);

/**
 * Call the string class.
 * @param ctxt The current running context.
 * @param o The string class object.
 * @param thiz This argument.
 * @param args Arguments.
 * @param argc Arguments' count.
 * @param[out] rv Return value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_string_class_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv);

/**
 * Call the function class.
 * @param ctxt The current running context.
 * @param o The function class object.
 * @param thiz This argument.
 * @param args Arguments.
 * @param argc Arguments' count.
 * @param[out] rv Return value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_function_class_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv);

/**
 * Initialize the package manager data.
 * @param ctxt The current running context.
 */
extern void
ox_package_init (OX_Context *ctxt);

/**
 * Release the package manager data.
 * @param ctxt The current running context.
 */
extern void
ox_package_deinit (OX_Context *ctxt);

/**
 * Scan the referenced objects in the packages.
 * @param ctxt The current running context.
 */
extern void
ox_gc_scan_package (OX_Context *ctxt);

/**
 * Convert the number to string.
 * @param ctxt The current running context.
 * @param n The number.
 * @param[out] s Return result string.
 * @param flags Convert flags. (OX_SOUT_XXXX)
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_number_to_string (OX_Context *ctxt, OX_Number n, OX_Value *s, int flags);

/**
 * Convert the integer number to string.
 * @param ctxt The current running context.
 * @param kind Kind of the integer.
 * @param i The integer number.
 * @param[out] s Return result string.
 * @param flags Convert flags. (OX_SOUT_XXXX)
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_int_to_string (OX_Context *ctxt, OX_CTypeKind kind, int64_t i, OX_Value *s, int flags);

/**
 * Convert the value to string with expected format.
 * @param ctxt The current running context.add_cmd
 * @param v The value.
 * @param[out] s Return the string.
 * @param flags Format flags.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_to_string_format (OX_Context *ctxt, OX_Value *v, OX_Value *s, int flags);

/**
 * Create a new match object.
 * @param ctxt The current running context.
 * @param[out] mv Return the match result object.
 * @param s The origin string.
 * @param start Start position of match.
 * @param end End position of match.
 * @param n_group Sub group number.
 * @param slices Slices of each group.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_match_new (OX_Context *ctxt, OX_Value *mv, OX_Value *s,
        size_t start, size_t end, int n_group, OX_Slice *slices);

/**
 * Get the match data from the match object.
 * @param ctxt The current running context.
 * @param v The match object value.
 * @return The match data.
 */
extern OX_Match*
ox_match_get_data (OX_Context *ctxt, OX_Value *v);

/**
 * Create a new dictionary.
 * @param ctxt The current running context.
 * @param[out] dict Return the new dictionary.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_dict_new (OX_Context *ctxt, OX_Value *dict);

/**
 * Add a new entry to the dictionary.
 * @param ctxt The current running context.
 * @param dict The dictionary.
 * @param k The key of the new entry.
 * @param v The value of the new entry.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_dict_add (OX_Context *ctxt, OX_Value *dict, OX_Value *k, OX_Value *v);

/**
 * Free the context.
 * @param ctxt The context to be freed.
 */
extern void
ox_context_free (OX_Context *ctxt);

/**
 * Initialize the architecture related resource.
 */
extern void
ox_arch_init (void);

/**
 * Release the architecture related resource.
 */
extern void
ox_arch_deinit (void);

/**
 * Get the current executable program's path.
 * @param[out] buf The path output buffer.
 * @param len The buffer's length.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_exe_path (char *buf, size_t len);

/**
 * Open a dynamic library.
 * @param ctxt The current running context.
 * @param name Name of the dynamic library.
 * @return The handle of the dynamic library.
 * @retval NULL On error.
 */
extern void*
ox_dl_open (OX_Context *ctxt, const char *name);

/**
 * Get the current thread's ID.
 * @return The current thread's ID.
 */
extern int
ox_thread_id (void);

/**
 * Create a new thread.
 * @param ctxt The current running context.
 * @param[out] th Return the new thread.
 * @param entry The entry function.
 * @param arg The argument of the entry function.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_thread_create (OX_Context *ctxt, OX_Thread *th, void *(entry)(void *arg), void *arg);

/**
 * Wait until the thread is end and release the thread's resource.
 * @param ctxt The current running context.
 * @param th The thread.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_thread_join (OX_Context *ctxt, OX_Thread *th);

/**
 * Set the thread as detached.
 * Detached means the thread's resource will be released by itself.
 * @param ctxt The current running context.
 * @param th The thread.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_thread_detach (OX_Context *ctxt, OX_Thread *th);

/**
 * Yield the thread.
 */
extern void
ox_thread_yield ();

/**
 * Initialize the thread key.
 * @param key The key to be initialized.
 */
extern void
ox_thread_key_init (OX_ThreadKey *key);

/**
 * Release the thread key.
 * @param key The key to be released.
 */
extern void
ox_thread_key_deinit (OX_ThreadKey *key);

/**
 * Get the thread key's value.
 * @param key The thread key.
 * @return The value of the key.
 */
extern void*
ox_thread_key_get (OX_ThreadKey *key);

/**
 * Set the thread key's value.
 * @param key The thread key.
 * @param v The new value of the key.
 */
extern void
ox_thread_key_set (OX_ThreadKey *key, void *v);

/**
 * Initialize the mutex.
 * @param mutex The mutex to be initialized.
 */
extern void
ox_mutex_init (OX_Mutex *mutex);

/**
 * Release the mutex.
 * @param mutex the mutex to be released.
 */
extern void
ox_mutex_deinit (OX_Mutex *mutex);

/**
 * Lock the mutex.
 * @param mutex The mutex.
 */
extern void
ox_mutex_lock (OX_Mutex *mutex);

/**
 * Unlock the mutex.
 * @param mutex The mutex.
 */
extern void
ox_mutex_unlock (OX_Mutex *mutex);

/**
 * Initialize the condition variable.
 * @param cv The condition varaible to be initialized.
 */
extern void
ox_cond_var_init (OX_CondVar *cv);

/**
 * Release the condition variable.
 * @param cv The condition varaible to be released.
 */
extern void
ox_cond_var_deinit (OX_CondVar *cv);

/**
 * Wakeup a thread waiting for the condition variable.
 * @param cv The condition variable.
 */
extern void
ox_cond_var_signal (OX_CondVar *cv);

/**
 * Suspend the current thread and wait for the conditon variable.
 * @param cv The condition variable.
 * @param mutex The mutex.
 * @param timeout_ms Waiting timeout in microseconds.
 * @retval OX_OK On success.
 * @retval OX_FALSE Timeout.
 */
extern OX_Result
ox_cond_var_wait (OX_CondVar *cv, OX_Mutex *mutex, int timeout_ms);

/**
 * Normalize the path string.
 * @param ctxt The current running context.
 * @param path The input path.
 * @param[out] s Return the string value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_path_to_str (OX_Context *ctxt, const char *path, OX_Value *s);

/**
 * Get the package lookup directories.
 * @param ctxt The current running context.
 * @param[out] dirs Return the directories array.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_package_get_dirs (OX_Context *ctxt, OX_Value *dirs);

/**
 * Convert the string template to string.
 * @param ctxt The current running context.
 * @param templ The string template.
 * @param[out] str The result string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_str_templ_to_str (OX_Context *ctxt, OX_Value *templ, OX_Value *str);

/**
 * Create the string template from a string.
 * @param ctxt The current running context.
 * @param[out] templ The string template.
 * @param str The result string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_str_templ_from_str (OX_Context *ctxt, OX_Value *templ, OX_Value *str);

/**
 * Check if 2 C types are equal.
 * @param ctxt The current running context.
 * @param t1 C type 1.
 * @param t2 C type 2.
 * @return 2 types are equal or not.
 */
extern OX_Bool
ox_ctype_equal (OX_Context *ctxt, OX_Value *t1, OX_Value *t2);

#ifdef __cplusplus
}
#endif

#endif
