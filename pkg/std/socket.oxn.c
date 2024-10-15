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
 * Socket.
 */

#define OX_LOG_TAG "socket"

#ifdef ARCH_LINUX
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/epoll.h>
#endif /*ARCH_LINUX*/

#ifdef ARCH_WIN
#include <winsock2.h>
#include <ws2tcpip.h>

typedef int socklen_t;
typedef u_short in_port_t;

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT 0
#endif

#endif /*ARCH_WIN*/

#include "std.h"

/*Declaration index.*/
enum {
    ID_Socket,
    ID_SockAddr,
    ID_SockMan,
    ID_sockaddr,
    ID_Socket_inf,
    ID_SockAddr_inf,
    ID_SockAddrInet_inf,
    ID_SockAddrInet6_inf,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "Socket",
    "SockAddr",
    "SockMan",
    "sockaddr",
    NULL
};

/*Script descrition.*/
static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/*Free the socket.*/
static void
socket_free (OX_Context *ctxt, void *p)
{
    int s = OX_PTR2SIZE(p);

    if (s != -1)
        close(s);
}

/*Socket's operation functions.*/
static const OX_PrivateOps
socket_ops = {
    NULL,
    socket_free
};

/** Socket address.*/
typedef struct {
    struct sockaddr addr; /**< Address.*/
    socklen_t       len;  /**< Length of address.*/
} OX_SockAddr;

/** Socket address's information.*/
typedef struct {
    int              family;   /**< Socket family.*/
    socklen_t        len;      /**< Length of the address.*/
    struct sockaddr *addr;     /**< Socket address.*/
    struct in_addr  *in_addr;  /**< Inet address.*/
    struct in6_addr *in6_addr; /**< Inet6 address.*/
} OX_SockAddrInfo;

/*Generate socket error.*/
static OX_Result
sock_error (OX_Context *ctxt, const char *fn)
{
#ifdef ARCH_WIN
    LPVOID lpMsgBuf;
    DWORD dw = WSAGetLastError();

    FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dw,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL);

    ox_throw_system_error(ctxt, OX_TEXT("\"%s\" failed: %d: %s"), fn, dw, lpMsgBuf);

    LocalFree(lpMsgBuf);
    return OX_ERR;
#else
    return std_system_error(ctxt, fn);
#endif
}

/*Check if it is timeout.*/
static OX_Bool
sock_is_timed_out ()
{
#ifdef ARCH_WIN
    int err = WSAGetLastError();

    if ((err == WSATRY_AGAIN) || (err == WSAEWOULDBLOCK) || (err == WSAEINPROGRESS) || (err == WSAETIMEDOUT))
        return OX_TRUE;
    else
        return OX_FALSE;
#else /* !defined ARCH_WIN */
    if ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINPROGRESS))
        return OX_TRUE;
    else
        return OX_FALSE;
#endif /*ARCH_WIN*/
}

/*Free the socket address.*/
static void
sock_addr_free (OX_Context *ctxt, void *p)
{
    OX_SockAddr *sa = p;

    OX_DEL(ctxt, sa);
}

/*Socket address's operation functions.*/
static const OX_PrivateOps
sock_addr_ops = {
    NULL,
    sock_addr_free
};

/** Socket manager.*/
typedef struct {
    OX_Value  evt_cb;    /**< Event callback function.*/
    OX_Hash   sock_hash; /**< Socket hash table.*/
#ifdef ARCH_LINUX
    int       epoll_fd;  /**< epoll device file descriptor.*/
#endif /*ARCH_LINUX*/
} OX_SockMan;

/** Socket event.*/
typedef enum {
    OX_SOCK_EVENT_IN  = (1 << 0),
    OX_SOCK_EVENT_OUT = (1 << 1),
    OX_SOCK_EVENT_ERR = (1 << 2)
} OX_SockEvent;

/** Socket hash entry.*/
typedef struct {
    OX_HashEntry he;    /**< Hash table entry.*/
    OX_Value     sockv; /**< The socket value.*/
    OX_SockEvent events;/**< Events flags.*/
} OX_SockEntry;

/*Free the socket entry.*/
static void
sock_entry_free (OX_Context *ctxt, OX_SockEntry *se)
{
    OX_DEL(ctxt, se);
}

/*Scan the referenced objects in the socket manager.*/
static void
sock_man_scan (OX_Context *ctxt, void *p)
{
    OX_SockMan *sm = p;
    size_t i;
    OX_SockEntry *se;

    ox_hash_foreach_c(&sm->sock_hash, i, se, OX_SockEntry, he) {
        ox_gc_scan_value(ctxt, &se->sockv);
    }

    ox_gc_scan_value(ctxt, &sm->evt_cb);
}

/*Free the socket manager.*/
static void
sock_man_free (OX_Context *ctxt, void *p)
{
    OX_SockMan *sm = p;
    size_t i;
    OX_SockEntry *se, *nse;

    /*Free the socket hash table.*/
    ox_hash_foreach_safe_c(&sm->sock_hash, i, se, nse, OX_SockEntry, he) {
        sock_entry_free(ctxt, se);
    }

    ox_hash_deinit(ctxt, &sm->sock_hash);

#ifdef ARCH_LINUX
    if (sm->epoll_fd != -1)
        close(sm->epoll_fd);
#endif /*ARCH_LINUX*/

    OX_DEL(ctxt, sm);
}

/*Socket manager operation functions.*/
static const OX_PrivateOps
sock_man_ops = {
    sock_man_scan,
    sock_man_free
};

/*Get the socket address from a value.*/
static OX_SockAddr*
sock_addr_get (OX_Context *ctxt, OX_Value *v)
{
    OX_SockAddr *sa;

    if (!(sa = ox_object_get_priv(ctxt, v, &sock_addr_ops)))
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a socket address"));

    return sa;
}

/*Get the inet4 socket address from a value.*/
static OX_SockAddr*
sock_addr_in_get (OX_Context *ctxt, OX_Value *v)
{
    OX_SockAddr *sa;

    if (!(sa = ox_object_get_priv(ctxt, v, &sock_addr_ops)))
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a socket address"));

    if (sa->addr.sa_family != AF_INET) {
        ox_throw_type_error(ctxt, OX_TEXT("the socket address is not an inet address"));
        sa = NULL;
    }

    return sa;
}

/*Get the inet6 socket address from a value.*/
static OX_SockAddr*
sock_addr_in6_get (OX_Context *ctxt, OX_Value *v)
{
    OX_SockAddr *sa;

    if (!(sa = ox_object_get_priv(ctxt, v, &sock_addr_ops)))
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a socket address"));

    if (sa->addr.sa_family != AF_INET6) {
        ox_throw_type_error(ctxt, OX_TEXT("the socket address is not an inet6 address"));
        sa = NULL;
    }

    return sa;
}

/*Allocate a new socket address.*/
static OX_Result
sock_addr_new (OX_Context *ctxt, OX_Value *v, OX_Value *f, OX_SockAddrInfo *info)
{
    OX_SockAddr *sa;
    OX_Value *inf;
    OX_Result r;

    if (!OX_NEW_0(ctxt, sa)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    if (info->addr) {
        memcpy(&sa->addr, info->addr, sizeof(struct sockaddr));
    } else if (info->family) {
        sa->addr.sa_family = info->family;
    }

    if (info->len) {
        sa->len = info->len;
    } else {
        if (sa->addr.sa_family == AF_INET) {
            sa->len = sizeof(struct sockaddr_in);
        } else if (sa->addr.sa_family == AF_INET6) {
            sa->len = sizeof(struct sockaddr_in6);
        } else {
            sa->len = sizeof(struct sockaddr);
        }
    }

    if (sa->addr.sa_family == AF_INET) {
        inf = ox_script_get_value(ctxt, f, ID_SockAddrInet_inf);

        if (!info->addr && info->in_addr) {
            struct sockaddr_in *sin = (struct sockaddr_in*)&sa->addr;

            memcpy(&sin->sin_addr, info->in_addr, sizeof(struct in_addr));
        }
    } else if (sa->addr.sa_family == AF_INET6) {
        inf = ox_script_get_value(ctxt, f, ID_SockAddrInet6_inf);

        if (!info->addr && info->in6_addr) {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)&sa->addr;

            memcpy(&sin6->sin6_addr, info->in6_addr, sizeof(struct in6_addr));
        }
    } else {
        inf = ox_script_get_value(ctxt, f, ID_SockAddr_inf);
    }

    if (ox_value_is_null(ctxt, v)) {
        if ((r = ox_object_new(ctxt, v, inf)) == OX_ERR)
            goto end;
    } else {
        if ((r = ox_object_set_interface(ctxt, v, inf)) == OX_ERR)
            goto end;
    }

    if ((r = ox_object_set_priv(ctxt, v, &sock_addr_ops, sa)) == OX_ERR) {
        sock_addr_free(ctxt, sa);
        goto end;
    }

    r = OX_OK;
end:
    return r;
}

/*Get the socket.*/
static OX_Result
socket_get (OX_Context *ctxt, OX_Value *v, int *ps)
{
    if (ox_object_get_priv_ops(ctxt, v) != &socket_ops) {
        return ox_throw_type_error(ctxt, OX_TEXT("the value is not a socket"));
    }

    *ps = OX_PTR2SIZE(ox_object_get_priv(ctxt, v, &socket_ops));
    return OX_OK;
}

/*Get the openned socket.*/
static int
openned_socket_get (OX_Context *ctxt, OX_Value *v)
{
    int s = -1;
    OX_Result r;

    if ((r = socket_get(ctxt, v, &s)) == OX_ERR)
        return -1;

    if (s == -1)
        ox_throw_reference_error(ctxt, OX_TEXT("the socket is closed"));

    return s;
}

/*Socket.$inf.$init.*/
static OX_Result
Socket_inf_init (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *domain_v = ox_argument(ctxt, args, argc, 0);
    OX_Value *type_v = ox_argument(ctxt, args, argc, 1);
    OX_Value *proto_v = ox_argument(ctxt, args, argc, 2);
    int32_t domain, type, proto;
    int s;
    OX_Result r;

    if ((r = ox_to_int32(ctxt, domain_v, &domain)) == OX_ERR)
        goto end;

    if ((r = ox_to_int32(ctxt, type_v, &type)) == OX_ERR)
        goto end;

    if ((r = ox_to_int32(ctxt, proto_v, &proto)) == OX_ERR)
        goto end;

    if ((s = socket(domain, type, proto)) == -1) {
        r = sock_error(ctxt, "socket");
        goto end;
    }

    if ((r = ox_object_set_priv(ctxt, thiz, &socket_ops, OX_SIZE2PTR(s))) == OX_ERR) {
        close(s);
        goto end;
    }

    r = OX_OK;
end:
    return r;
}

/*Socket.$inf.connect.*/
static OX_Result
Socket_inf_connect (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *addr_v = ox_argument(ctxt, args, argc, 0);
    OX_SockAddr *sa;
    OX_Result r;
    OX_Bool b = OX_TRUE;
    int s;

    if ((s = openned_socket_get(ctxt, thiz)) == -1) {
        r = OX_ERR;
        goto end;
    }

    if (!(sa = sock_addr_get(ctxt, addr_v))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = connect(s, &sa->addr, sa->len)) == -1) {
        if (sock_is_timed_out()) {
            b = OX_FALSE;
        } else {
            r = sock_error(ctxt, "connect");
            goto end;
        }
    }

    ox_value_set_bool(ctxt, rv, b);
    r = OX_OK;
end:
    return r;
}

/*Socket.$inf.bind.*/
static OX_Result
Socket_inf_bind (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *addr_v = ox_argument(ctxt, args, argc, 0);
    OX_SockAddr *sa;
    OX_Result r;
    int s;

    if ((s = openned_socket_get(ctxt, thiz)) == -1) {
        r = OX_ERR;
        goto end;
    }

    if (!(sa = sock_addr_get(ctxt, addr_v))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = bind(s, &sa->addr, sa->len)) == -1) {
        r = sock_error(ctxt, "bind");
        goto end;
    }

    r = OX_OK;
end:
    return r;
}

/*Socket.$inf.accept.*/
static OX_Result
Socket_inf_accept (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *addr_v = ox_argument(ctxt, args, argc, 0);
    OX_Value *inf;
    OX_SockAddr *sa;
    OX_Result r;
    int s, rs;

    if ((s = openned_socket_get(ctxt, thiz)) == -1) {
        r = OX_ERR;
        goto end;
    }

    if (!(sa = sock_addr_get(ctxt, addr_v))) {
        r = OX_ERR;
        goto end;
    }

    if ((rs = accept(s, &sa->addr, &sa->len)) == -1) {
        r = sock_error(ctxt, "accept");
        goto end;
    }

    inf = ox_script_get_value(ctxt, f, ID_Socket_inf);

    if ((r = ox_object_new(ctxt, rv, inf)) == OX_ERR)
        goto end;

    if ((r = ox_object_set_priv(ctxt, rv, &socket_ops, OX_SIZE2PTR(rs))) == OX_ERR) {
        close(rs);
        goto end;
    }

    r = OX_OK;
end:
    return r;
}

/*Socket.$inf.listen.*/
static OX_Result
Socket_inf_listen (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *backlog_v = ox_argument(ctxt, args, argc, 0);
    int32_t backlog;
    OX_Result r;
    int s;

    if ((s = openned_socket_get(ctxt, thiz)) == -1) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_int32(ctxt, backlog_v, &backlog)) == OX_ERR)
        goto end;

    if ((r = listen(s, backlog)) == -1) {
        r = sock_error(ctxt, "listen");
        goto end;
    }

    r = OX_OK;
end:
    return r;
}

/*Socket.$inf.recv.*/
static OX_Result
Socket_inf_recv (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *buf = ox_argument(ctxt, args, argc, 0);
    OX_Value *flags_v = ox_argument(ctxt, args, argc, 1);
    OX_Value *start_v = ox_argument(ctxt, args, argc, 2);
    OX_Value *len_v = ox_argument(ctxt, args, argc, 3);
    OX_Value *cty;
    OX_CArrayInfo cai;
    int32_t flags;
    size_t start = 0, len = 0;
    ssize_t rn;
    OX_Result r;
    int s;

    if ((s = openned_socket_get(ctxt, thiz)) == -1) {
        r = OX_ERR;
        goto end;
    }

    if (!ox_value_is_cvalue(ctxt, buf)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a C value"));
        goto end;
    }

    if ((r = ox_to_int32(ctxt, flags_v, &flags)) == OX_ERR)
        goto end;

    cty = ox_cvalue_get_ctype(ctxt, buf);
    if ((r = ox_ctype_get_array_info(ctxt, cty, &cai)) == OX_ERR)
        goto end;

    if (cai.isize == -1) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the C array's item size is unknown"));
        goto end;
    }

    if (!ox_value_is_null(ctxt, start_v)) {
        if ((r = ox_to_index(ctxt, start_v, &start)) == OX_ERR)
            goto end;
    } else {
        start = 0;
    }

    if (!ox_value_is_null(ctxt, len_v)) {
        if ((r = ox_to_index(ctxt, len_v, &len)) == OX_ERR)
            goto end;
    } else {
        if (cai.len == -1) {
            r = ox_throw_type_error(ctxt, OX_TEXT("the C array's length is unknown"));
            goto end;
        }

        if (start < cai.len)
            len = cai.len - start;
        else
            len = 0;
    }

    if (start >= cai.len) {
        rn = 0;
    } else {
        char *b;

        if (start + len > cai.len)
            len = cai.len - start;

        b = ox_cvalue_get_pointer(ctxt, buf);

        rn = recv(s, b + start * cai.isize, len * cai.isize, flags);
        if (rn == -1) {
            if (sock_is_timed_out()) {
                rn = 0;
            } else {
                r = sock_error(ctxt, "recv");
                goto end;
            }
        }
    }

    ox_value_set_number(ctxt, rv, rn / cai.isize);
    r = OX_OK;
end:
    return r;
}

/*Socket.$inf.send.*/
static OX_Result
Socket_inf_send (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *buf = ox_argument(ctxt, args, argc, 0);
    OX_Value *flags_v = ox_argument(ctxt, args, argc, 1);
    OX_Value *start_v = ox_argument(ctxt, args, argc, 2);
    OX_Value *len_v = ox_argument(ctxt, args, argc, 3);
    OX_VS_PUSH(ctxt, str)
    OX_Value *cty;
    OX_CArrayInfo cai;
    int32_t flags;
    size_t start = 0, len = 0, isize;
    ssize_t rn;
    const char *send_p = NULL;
    size_t send_len = 0;
    OX_Result r;
    int s;

    if ((s = openned_socket_get(ctxt, thiz)) == -1) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_int32(ctxt, flags_v, &flags)) == OX_ERR)
        goto end;

    if (!ox_value_is_cvalue(ctxt, buf)) {
        if ((r = ox_to_string(ctxt, buf, str)) == OX_ERR)
            goto end;

        send_p = ox_string_get_char_star(ctxt, str);
        send_len = ox_string_length(ctxt, str);
        isize = 1;
    } else {
        cty = ox_cvalue_get_ctype(ctxt, buf);
        if ((r = ox_ctype_get_array_info(ctxt, cty, &cai)) == OX_ERR)
            goto end;

        if (cai.isize == -1) {
            r = ox_throw_type_error(ctxt, OX_TEXT("the C array's item size is unknown"));
            goto end;
        }

        if (!ox_value_is_null(ctxt, start_v)) {
            if ((r = ox_to_index(ctxt, start_v, &start)) == OX_ERR)
                goto end;
        } else {
            start = 0;
        }

        if (!ox_value_is_null(ctxt, len_v)) {
            if ((r = ox_to_index(ctxt, len_v, &len)) == OX_ERR)
                goto end;
        } else {
            if (cai.len == -1) {
                r = ox_throw_type_error(ctxt, OX_TEXT("the C array's length is unknown"));
                goto end;
            }

            if (start < cai.len)
                len = cai.len - start;
            else
                len = 0;
        }

        if (start < cai.len) {
            char *b = ox_cvalue_get_pointer(ctxt, buf);

            send_p = b + start * cai.isize;

            if (start + len > cai.len)
                len = cai.len - start;

            send_len = len * cai.isize;
        }

        isize = cai.isize;
    }

    if (send_len == 0) {
        rn = 0;
    } else {
        rn = send(s, send_p, send_len, flags);
        if (rn == -1) {
            if (sock_is_timed_out()) {
                rn = 0;
            } else {
                r = sock_error(ctxt, "send");
                goto end;
            }
        }
    }

    ox_value_set_number(ctxt, rv, rn / isize);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, str)
    return r;
}

/*Socket.$inf.recvfrom.*/
static OX_Result
Socket_inf_recvfrom (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *buf = ox_argument(ctxt, args, argc, 0);
    OX_Value *flags_v = ox_argument(ctxt, args, argc, 1);
    OX_Value *addr_v = ox_argument(ctxt, args, argc, 2);
    OX_Value *start_v = ox_argument(ctxt, args, argc, 3);
    OX_Value *len_v = ox_argument(ctxt, args, argc, 4);
    OX_SockAddr *sa;
    OX_Value *cty;
    OX_CArrayInfo cai;
    int32_t flags;
    size_t start = 0, len = 0;
    ssize_t rn;
    OX_Result r;
    int s;

    if ((s = openned_socket_get(ctxt, thiz)) == -1) {
        r = OX_ERR;
        goto end;
    }

    if (!ox_value_is_cvalue(ctxt, buf)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a C value"));
        goto end;
    }

    if ((r = ox_to_int32(ctxt, flags_v, &flags)) == OX_ERR)
        goto end;

    if (!(sa = sock_addr_get(ctxt, addr_v))) {
        r = OX_ERR;
        goto end;
    }

    cty = ox_cvalue_get_ctype(ctxt, buf);
    if ((r = ox_ctype_get_array_info(ctxt, cty, &cai)) == OX_ERR)
        goto end;

    if (cai.isize == -1) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the C array's item size is unknown"));
        goto end;
    }

    if (!ox_value_is_null(ctxt, start_v)) {
        if ((r = ox_to_index(ctxt, start_v, &start)) == OX_ERR)
            goto end;
    } else {
        start = 0;
    }

    if (!ox_value_is_null(ctxt, len_v)) {
        if ((r = ox_to_index(ctxt, len_v, &len)) == OX_ERR)
            goto end;
    } else {
        if (cai.len == -1) {
            r = ox_throw_type_error(ctxt, OX_TEXT("the C array's length is unknown"));
            goto end;
        }

        if (start < cai.len)
            len = cai.len - start;
        else
            len = 0;
    }

    if (start >= cai.len) {
        rn = 0;
    } else {
        char *b;

        if (start + len > cai.len)
            len = cai.len - start;

        b = ox_cvalue_get_pointer(ctxt, buf);
        rn = recvfrom(s, b + start * cai.isize, len * cai.isize, flags, &sa->addr, &sa->len);
        if (rn == -1) {
            if (sock_is_timed_out()) {
                rn = 0;
            } else {
                r = sock_error(ctxt, "recvfrom");
                goto end;
            }
        }
    }

    ox_value_set_number(ctxt, rv, rn / cai.isize);
    r = OX_OK;
end:
    return r;
}

/*Socket.$inf.sendto.*/
static OX_Result
Socket_inf_sendto (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *buf = ox_argument(ctxt, args, argc, 0);
    OX_Value *flags_v = ox_argument(ctxt, args, argc, 1);
    OX_Value *addr_v = ox_argument(ctxt, args, argc, 2);
    OX_Value *start_v = ox_argument(ctxt, args, argc, 3);
    OX_Value *len_v = ox_argument(ctxt, args, argc, 4);
    OX_VS_PUSH(ctxt, str)
    OX_SockAddr *sa;
    OX_Value *cty;
    OX_CArrayInfo cai;
    int32_t flags;
    size_t start = 0, len = 0, isize;
    ssize_t rn;
    const char *send_p = NULL;
    size_t send_len = 0;
    OX_Result r;
    int s;

    if ((s = openned_socket_get(ctxt, thiz)) == -1) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_int32(ctxt, flags_v, &flags)) == OX_ERR)
        goto end;

    if (!(sa = sock_addr_get(ctxt, addr_v))) {
        r = OX_ERR;
        goto end;
    }

    if (!ox_value_is_cvalue(ctxt, buf)) {
        if ((r = ox_to_string(ctxt, buf, str)) == OX_ERR)
            goto end;

        send_p = ox_string_get_char_star(ctxt, str);
        send_len = ox_string_length(ctxt, str);
        isize = 1;
    } else {
        cty = ox_cvalue_get_ctype(ctxt, buf);
        if ((r = ox_ctype_get_array_info(ctxt, cty, &cai)) == OX_ERR)
            goto end;

        if (cai.isize == -1) {
            r = ox_throw_type_error(ctxt, OX_TEXT("the C array's item size is unknown"));
            goto end;
        }

        if (!ox_value_is_null(ctxt, start_v)) {
            if ((r = ox_to_index(ctxt, start_v, &start)) == OX_ERR)
                goto end;
        } else {
            start = 0;
        }

        if (!ox_value_is_null(ctxt, len_v)) {
            if ((r = ox_to_index(ctxt, len_v, &len)) == OX_ERR)
                goto end;
        } else {
            if (cai.len == -1) {
                r = ox_throw_type_error(ctxt, OX_TEXT("the C array's length is unknown"));
                goto end;
            }

            if (start < cai.len)
                len = cai.len - start;
            else
                len = 0;
        }

        if (start < cai.len) {
            char *b = ox_cvalue_get_pointer(ctxt, buf);

            send_p = b + start * cai.isize;

            if (start + len > cai.len)
                len = cai.len - start;

            send_len = len * cai.isize;
        }

        isize = cai.isize;
    }

    if (send_len == 0) {
        rn = 0;
    } else {
        rn = sendto(s, send_p, send_len, flags, &sa->addr, sa->len);
        if (rn == -1) {
            if (sock_is_timed_out()) {
                rn = 0;
            } else {
                r = sock_error(ctxt, "sendto");
                goto end;
            }
        }
    }

    ox_value_set_number(ctxt, rv, rn / isize);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, str)
    return r;
}

/*Socket.$inf.$close.*/
static OX_Result
Socket_inf_close (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    int s;
    OX_Result r;

    if ((r = socket_get(ctxt, thiz, &s)) == OX_ERR)
        goto end;

    if (s != -1) {
        close(s);

        if ((r = ox_object_set_priv(ctxt, thiz, &socket_ops, OX_SIZE2PTR(-1))) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    return r;
}

/*Socket.$inf.set_recv_timeout.*/
static OX_Result
Socket_inf_set_recv_timeout (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n = -1;
    int s;
    OX_Result r;
#ifdef ARCH_WIN
    DWORD timeout, *ptimeout;
#else
    struct timeval timeout, *ptimeout;
#endif

    if (!(s = openned_socket_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if (argc > 0) {
        if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
            goto end;
    }

#ifdef ARCH_WIN
    if ((n < 0) || isinf(n) || isnan(n)) {
        timeout = 0;
    } else {
        timeout = n;
    }

    ptimeout = &timeout;
#else /* !defined ARCH_WIN */
    if ((n < 0) || isinf(n) || isnan(n)) {
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
    } else {
        int ms = n;

        timeout.tv_sec = ms / 1000;
        timeout.tv_usec = (ms % 1000) * 1000;
    }

    ptimeout = &timeout;
#endif /*ARCH_WIN*/

    if ((r = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (void*)ptimeout, sizeof(timeout))) == -1) {
        r = sock_error(ctxt, "setsockopt");
        goto end;
    }

    r = OX_OK;
end:
    return r;
}

/*Socket.$inf.set_send_timeout.*/
static OX_Result
Socket_inf_set_send_timeout (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n = -1;
    int s;
    OX_Result r;
#ifdef ARCH_WIN
    DWORD timeout, *ptimeout;
#else
    struct timeval timeout, *ptimeout;
#endif

    if (!(s = openned_socket_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if (argc > 0) {
        if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
            goto end;
    }

#ifdef ARCH_WIN
    if ((n < 0) || isinf(n) || isnan(n)) {
        timeout = 0;
    } else {
        timeout = n;
    }

    ptimeout = &timeout;
#else /* !defined ARCH_WIN */
    if ((n < 0) || isinf(n) || isnan(n)) {
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
    } else {
        int ms = n;

        timeout.tv_sec = ms / 1000;
        timeout.tv_usec = (ms % 1000) * 1000;
    }

    ptimeout = &timeout;
#endif /*ARCH_WIN*/

    if ((r = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (void*)ptimeout, sizeof(timeout))) == -1) {
        r = sock_error(ctxt, "setsockopt");
        goto end;
    }

    r = OX_OK;
end:
    return r;
}

/*Socket.$inf.add_membership.*/
static OX_Result
Socket_inf_add_membership (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, str)
    const char *p;
    struct ip_mreq group;
    int s;
    OX_Result r;

    if (!(s = openned_socket_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if (ox_value_is_number(ctxt, arg)) {
        int32_t addr;

        if ((r = ox_to_int32(ctxt, arg, &addr)) == OX_ERR)
            goto end;

        group.imr_multiaddr.s_addr = htonl(addr);
    } else {
        if ((r = ox_to_string(ctxt, arg, str)) == OX_ERR)
            goto end;

        p = ox_string_get_char_star(ctxt, str);
        r = inet_pton(AF_INET, p, &group.imr_multiaddr.s_addr);
        if (r == -1) {
            r = sock_error(ctxt, "inet_pton");
            goto end;
        } else if (r == 0) {
            r = ox_throw_type_error(ctxt, OX_TEXT("argument is not a valid inet address"));
            goto end;
        }
    }

    group.imr_interface.s_addr = htonl(INADDR_ANY);

    if ((r = setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&group, sizeof(group))) == -1) {
        r = sock_error(ctxt, "setsockopt");
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, str)
    return r;
}

/*Socket.$inf.set_nonblock.*/
static OX_Result
Socket_inf_set_nonblock (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Bool b;
    int s;
    int r;

    if (!(s = openned_socket_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    b = ox_to_bool(ctxt, arg);

#ifdef ARCH_WIN
    {
        unsigned long ul = b ? 1 : 0;

        if ((r = ioctlsocket(s, FIONBIO, (unsigned long*)&ul)) == -1) {
            r = sock_error(ctxt, "ioctlsocket");
            goto end;
        }
    }
#else /*!defined ARCH_WIN*/
    {
        int flags = fcntl(s, F_GETFL);

        if (b)
            flags |= O_NONBLOCK;
        else
            flags &= ~O_NONBLOCK;

        if ((r = fcntl(s, F_SETFL, flags)) == -1) {
            r = sock_error(ctxt, "fcntl");
            goto end;
        }
    }
#endif /*ARCH_WIN*/

    r = OX_OK;
end:
    return r;
}

/*Socket.$inf.fd get.*/
static OX_Result
Socket_inf_fd_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    int s;
    OX_Result r;

    if (!(s = openned_socket_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    ox_value_set_number(ctxt, rv, s);
    r = OX_OK;
end:
    return r;
}

/*SockAddr.from_c.*/
static OX_Result
SockAddr_from_c (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *cty;
    struct sockaddr *csa;
    OX_SockAddrInfo ai;
    OX_Result r;

    cty = ox_script_get_value(ctxt, f, ID_sockaddr);

    if (!ox_value_is_cptr(ctxt, arg, cty)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a \"struct sockaddr\""));
        goto end;
    }

    csa = ox_cvalue_get_pointer(ctxt, arg);

    memset(&ai, 0, sizeof(ai));
    ai.addr = csa;

    r = sock_addr_new(ctxt, rv, f, &ai);
end:
    return r;
}

/*SockAddr.lookup.*/
static OX_Result
SockAddr_lookup (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *name_v = ox_argument(ctxt, args, argc, 0);
    OX_Value *family_v = ox_argument(ctxt, args, argc, 1);
    int32_t family = AF_INET;
    OX_VS_PUSH_2(ctxt, str, sa)
    const char *name;
    struct addrinfo hints;
    struct addrinfo *result = NULL, *rp;
    OX_Result r;

    if ((r = ox_to_string(ctxt, name_v, str)) == OX_ERR)
        goto end;

    name = ox_string_get_char_star(ctxt, str);

    if (!ox_value_is_null(ctxt, family_v)) {
        if ((r = ox_to_int32(ctxt, family_v, &family)) == OX_ERR)
            goto end;
    }

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = family;

    r = getaddrinfo(name, NULL, &hints, &result);
    if (r != 0) {
        r = ox_throw_system_error(ctxt, OX_TEXT("\"%s\" failed: %s"),
                "getaddrinfo", gai_strerror(r));
        goto end;
    }

    if ((r = ox_array_new(ctxt, rv, 0)) == OX_ERR)
        goto end;

    for (rp = result; rp; rp = rp->ai_next) {
        OX_SockAddrInfo ai;

        memset(&ai, 0, sizeof(ai));

        ai.family = rp->ai_family;
        ai.addr = rp->ai_addr;
        ai.len = rp->ai_addrlen;

        if ((r = sock_addr_new(ctxt, sa, f, &ai)) == OX_ERR)
            goto end;

        if ((r = ox_array_append(ctxt, rv, sa)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    if (result)
        freeaddrinfo(result);
    OX_VS_POP(ctxt, str)
    return r;
}

/*SockAddr.$inf.$init.*/
static OX_Result
SockAddr_inf_init (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *family_v = ox_argument(ctxt, args, argc, 0);
    int32_t family = AF_INET;
    OX_SockAddrInfo ai;
    OX_Result r;

    if ((r = ox_to_int32(ctxt, family_v, &family)) == OX_ERR)
        goto end;

    memset(&ai, 0, sizeof(ai));
    ai.family = family;

    if ((r = sock_addr_new(ctxt, thiz, f, &ai)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    return r;
}

/*SockAddr.$inf.to_c.*/
static OX_Result
SockAddr_inf_to_c (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *cty;
    OX_SockAddr *sa;
    OX_CValueInfo cvi;
    OX_Result r;

    if (!(sa = sock_addr_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    cty = ox_script_get_value(ctxt, f, ID_SockAddr);

    cvi.v.p = &sa->addr;
    cvi.base = thiz;
    cvi.own = OX_CPTR_NON_OWNER;

    r = ox_cvalue_new(ctxt, rv, cty, &cvi);
end:
    return r;
}

/*SockAddr.$inf.length get.*/
static OX_Result
SockAddr_inf_length_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_SockAddr *sa;
    OX_Result r;

    if (!(sa = sock_addr_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    ox_value_set_number(ctxt, rv, sa->len);
    r = OX_OK;
end:
    return r;
}

/*SockAddr.$inf.family get.*/
static OX_Result
SockAddr_inf_family_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_SockAddr *sa;
    OX_Result r;

    if (!(sa = sock_addr_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    ox_value_set_number(ctxt, rv, sa->addr.sa_family);
    r = OX_OK;
end:
    return r;
}

/*SockAddrInet.$inf.port get.*/
static OX_Result
SockAddrInet_inf_port_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_SockAddr *sa;
    struct sockaddr_in *ia;
    OX_Result r;

    if (!(sa = sock_addr_in_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    ia = (struct sockaddr_in*)&sa->addr;

    ox_value_set_number(ctxt, rv, ntohs(ia->sin_port));
    r = OX_OK;
end:
    return r;
}

/*SockAddrInet.$inf.port set.*/
static OX_Result
SockAddrInet_inf_port_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    in_port_t port;
    OX_SockAddr *sa;
    struct sockaddr_in *ia;
    OX_Result r;

    if (!(sa = sock_addr_in_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_uint16(ctxt, arg, &port)) == OX_ERR)
        goto end;

    ia = (struct sockaddr_in*)&sa->addr;
    ia->sin_port = htons(port);
    r = OX_OK;
end:
    return r;
}

/*SockAddrInet.$inf.addr get.*/
static OX_Result
SockAddrInet_inf_addr_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_SockAddr *sa;
    struct sockaddr_in *ia;
    char buf[32];
    const char *p;
    OX_Result r;

    if (!(sa = sock_addr_in_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    ia = (struct sockaddr_in*)&sa->addr;
    p = inet_ntop(ia->sin_family, &ia->sin_addr, buf, sizeof(buf));
    if (!p) {
        r = sock_error(ctxt, "inet_ntop");
        goto end;
    }
    
    r = ox_string_from_char_star(ctxt, rv, buf);
end:
    return r;
}

/*SockAddrInet.$inf.addr set.*/
static OX_Result
SockAddrInet_inf_addr_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, str)
    const char *p;
    OX_SockAddr *sa;
    struct sockaddr_in *ia;
    OX_Result r;

    if (!(sa = sock_addr_in_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    ia = (struct sockaddr_in*)&sa->addr;

    if (ox_value_is_number(ctxt, arg)) {
        int32_t addr;

        if ((r = ox_to_int32(ctxt, arg, &addr)) == OX_ERR)
            goto end;

        ia->sin_addr.s_addr = htonl(addr);
    } else {
        if ((r = ox_to_string(ctxt, arg, str)) == OX_ERR)
            goto end;

        p = ox_string_get_char_star(ctxt, str);
        r = inet_pton(ia->sin_family, p, &ia->sin_addr);
        if (r == -1) {
            r = sock_error(ctxt, "inet_pton");
            goto end;
        } else if (r == 0) {
            r = ox_throw_type_error(ctxt, OX_TEXT("argument is not a valid inet address"));
            goto end;
        }
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, str)
    return r;
}

/*SockAddrInet6.$inf.port get.*/
static OX_Result
SockAddrInet6_inf_port_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_SockAddr *sa;
    struct sockaddr_in6 *ia;
    OX_Result r;

    if (!(sa = sock_addr_in6_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    ia = (struct sockaddr_in6*)&sa->addr;

    ox_value_set_number(ctxt, rv, ntohs(ia->sin6_port));
    r = OX_OK;
end:
    return r;
}

/*SockAddrInet6.$inf.port set.*/
static OX_Result
SockAddrInet6_inf_port_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    in_port_t port;
    OX_SockAddr *sa;
    struct sockaddr_in6 *ia;
    OX_Result r;

    if (!(sa = sock_addr_in6_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_uint16(ctxt, arg, &port)) == OX_ERR)
        goto end;

    ia = (struct sockaddr_in6*)&sa->addr;
    ia->sin6_port = htons(port);
    r = OX_OK;
end:
    return r;
}

/*SockAddrInet6.$inf.addr get.*/
static OX_Result
SockAddrInet6_inf_addr_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_SockAddr *sa;
    struct sockaddr_in6 *ia;
    char buf[64];
    const char *p;
    OX_Result r;

    if (!(sa = sock_addr_in6_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    ia = (struct sockaddr_in6*)&sa->addr;
    p = inet_ntop(ia->sin6_family, &ia->sin6_addr, buf, sizeof(buf));
    if (!p) {
        r = sock_error(ctxt, "inet_ntop");
        goto end;
    }
    
    r = ox_string_from_char_star(ctxt, rv, buf);
end:
    return r;
}

/*SockAddrInet6.$inf.addr set.*/
static OX_Result
SockAddrInet6_inf_addr_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, str)
    const char *p;
    OX_SockAddr *sa;
    struct sockaddr_in6 *ia;
    OX_Result r;

    if (!(sa = sock_addr_in6_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_string(ctxt, arg, str)) == OX_ERR)
        goto end;

    p = ox_string_get_char_star(ctxt, str);

    ia = (struct sockaddr_in6*)&sa->addr;
    r = inet_pton(ia->sin6_family, p, &ia->sin6_addr);
    if (r == -1) {
        r = sock_error(ctxt, "inet_pton");
        goto end;
    } else if (r == 0) {
        r = ox_throw_type_error(ctxt, OX_TEXT("argument is not a valid inet6 address"));
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, str)
    return r;
}

/*SockAddrInet6.$inf.flowinfo get.*/
static OX_Result
SockAddrInet6_inf_flowinfo_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_SockAddr *sa;
    struct sockaddr_in6 *ia;
    OX_Result r;

    if (!(sa = sock_addr_in6_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    ia = (struct sockaddr_in6*)&sa->addr;

    ox_value_set_number(ctxt, rv, ntohs(ia->sin6_flowinfo));
    r = OX_OK;
end:
    return r;
}

/*SockAddrInet6.$inf.flowinfo set.*/
static OX_Result
SockAddrInet6_inf_flowinfo_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    uint32_t flowinfo;
    OX_SockAddr *sa;
    struct sockaddr_in6 *ia;
    OX_Result r;

    if (!(sa = sock_addr_in6_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_uint32(ctxt, arg, &flowinfo)) == OX_ERR)
        goto end;

    ia = (struct sockaddr_in6*)&sa->addr;
    ia->sin6_flowinfo = htonl(flowinfo);
    r = OX_OK;
end:
    return r;
}

/*SockAddrInet6.$inf.scope_id get.*/
static OX_Result
SockAddrInet6_inf_scope_id_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_SockAddr *sa;
    struct sockaddr_in6 *ia;
    OX_Result r;

    if (!(sa = sock_addr_in6_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    ia = (struct sockaddr_in6*)&sa->addr;

    ox_value_set_number(ctxt, rv, ntohs(ia->sin6_scope_id));
    r = OX_OK;
end:
    return r;
}

/*SockAddrInet6.$inf.scope_id set.*/
static OX_Result
SockAddrInet6_inf_scope_id_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    uint32_t scope_id;
    OX_SockAddr *sa;
    struct sockaddr_in6 *ia;
    OX_Result r;

    if (!(sa = sock_addr_in6_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_uint32(ctxt, arg, &scope_id)) == OX_ERR)
        goto end;

    ia = (struct sockaddr_in6*)&sa->addr;
    ia->sin6_flowinfo = htonl(scope_id);
    r = OX_OK;
end:
    return r;
}

/*SockMan.$inf.$init.*/
static OX_Result
SockMan_inf_init (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *cb = ox_argument(ctxt, args, argc, 0);
    OX_SockMan *sm = NULL;
    OX_Result r;

    if (!ox_value_is_function(ctxt, cb)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a function"));
        goto end;
    }

    if (!(OX_NEW(ctxt, sm))) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

#ifdef ARCH_LINUX
    if ((sm->epoll_fd = epoll_create1(0)) == -1) {
        r = std_system_error(ctxt, "epoll_create1");
        goto end;
    }
#endif /*ARCH_LINUX*/

    ox_size_hash_init(&sm->sock_hash);
    ox_value_copy(ctxt, &sm->evt_cb, cb);

    if ((r = ox_object_set_priv(ctxt, thiz, &sock_man_ops, sm)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    if (r == OX_ERR) {
        if (sm)
            sock_man_free(ctxt, sm);
    }
    return r;
}

/*Get the socket manager from the value.*/
static OX_SockMan*
sock_man_get (OX_Context *ctxt, OX_Value *v)
{
    OX_SockMan *sm = ox_object_get_priv(ctxt, v, &sock_man_ops);

    if (!sm)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a socket manager"));

    return sm;
}

/*SockMan.$inf.add.*/
static OX_Result
SockMan_inf_add (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *sock_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *evt_arg = ox_argument(ctxt, args, argc, 1);
    OX_SockMan *sm = NULL;
    int sock;
    uint32_t iflags;
    OX_HashEntry **phe;
    OX_SockEntry *se;
    OX_Result r;
    OX_Bool add = OX_FALSE;
#ifdef ARCH_LINUX
    struct epoll_event evt;
#endif /*ARCH_LINUX*/

    if (!(sm = sock_man_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = socket_get(ctxt, sock_arg, &sock)) == OX_ERR)
        goto end;

    if ((r = ox_to_uint32(ctxt, evt_arg, &iflags)) == OX_ERR)
        goto end;

    se = ox_hash_lookup_c(ctxt, &sm->sock_hash, OX_SIZE2PTR(sock), &phe, OX_SockEntry, he);
    if (!se) {
        add = OX_TRUE;

        if (!OX_NEW(ctxt, se)) {
            r = ox_throw_no_mem_error(ctxt);
            goto end;
        }

        ox_value_copy(ctxt, &se->sockv, sock_arg);

        if ((r = ox_hash_insert(ctxt, &sm->sock_hash, OX_SIZE2PTR(sock), &se->he, phe)) == OX_ERR)
            goto end;
    }

#ifdef ARCH_WIN
    se->events = iflags;
#endif /*ARCH_WIN*/

#ifdef ARCH_LINUX
    evt.events = EPOLLHUP|EPOLLRDHUP;

    if (iflags & OX_SOCK_EVENT_IN)
        evt.events |= EPOLLIN;
    if (iflags & OX_SOCK_EVENT_OUT)
        evt.events |= EPOLLOUT;
    if (iflags & OX_SOCK_EVENT_ERR)
        evt.events |= EPOLLERR;

    evt.data.fd = sock;

    if ((r = epoll_ctl(sm->epoll_fd, add ? EPOLL_CTL_ADD : EPOLL_CTL_MOD, sock, &evt)) == -1) {
        r = std_system_error(ctxt, "epoll_ctl");
        goto end;
    }
#endif /*ARCH_LINUX*/

    r = OX_OK;
end:
    if ((r == OX_ERR) && add) {
        if (se) {
            ox_hash_remove(ctxt, &sm->sock_hash, OX_SIZE2PTR(sock), NULL);
            sock_entry_free(ctxt, se);
        }
    }
    return r;
}

/*SockMan.$inf.remove.*/
static OX_Result
SockMan_inf_remove (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *sock_arg = ox_argument(ctxt, args, argc, 0);
    OX_SockMan *sm;
    int sock;
    OX_Result r;
    OX_HashEntry **phe;
    OX_SockEntry *se;

    if (!(sm = sock_man_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = socket_get(ctxt, sock_arg, &sock)) == OX_ERR)
        goto end;

    se = ox_hash_lookup_c(ctxt, &sm->sock_hash, OX_SIZE2PTR(sock), &phe, OX_SockEntry, he);
    if (se) {
#ifdef ARCH_LINUX
        if ((r = epoll_ctl(sm->epoll_fd, EPOLL_CTL_DEL, sock, NULL)) == -1) {
            r = std_system_error(ctxt, "epoll_ctl");
            goto end;
        }
#endif /*ARCH_LINUX*/

        ox_hash_remove(ctxt, &sm->sock_hash, OX_SIZE2PTR(sock), phe);
        sock_entry_free(ctxt, se);
    }

    r = OX_OK;
end:
    return r;
}

/*SockMan.$inf.process.*/
static OX_Result
SockMan_inf_process (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *timeout_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH_3(ctxt, sock, event, cbr)
    int32_t timeout = -1;
    OX_SockMan *sm;
    OX_Result r;
#ifdef ARCH_WIN
    fd_set rd_fds, wr_fds, err_fds;
    int max_fd = -1;
    OX_SockEntry *se;
    size_t i;
#endif /*ARCH_WIN*/
#ifdef ARCH_LINUX
    int i;
    struct epoll_event evts[64];
#endif /*ARCH_LINUX*/

    if (!(sm = sock_man_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if (!ox_value_is_null(ctxt, timeout_arg)) {
        if ((r = ox_to_int32(ctxt, timeout_arg, &timeout)) == OX_ERR)
            goto end;
    }

#ifdef ARCH_WIN
    FD_ZERO(&rd_fds);
    FD_ZERO(&wr_fds);
    FD_ZERO(&err_fds);

    ox_hash_foreach_c(&sm->sock_hash, i, se, OX_SockEntry, he) {
        int fd = OX_PTR2SIZE(se->he.key);

        max_fd = OX_MAX(max_fd, fd);

        if (se->events & OX_SOCK_EVENT_IN)
            FD_SET(fd, &rd_fds);
        if (se->events & OX_SOCK_EVENT_OUT)
            FD_SET(fd, &wr_fds);

        FD_SET(fd, &err_fds);
    }

    if (max_fd != -1) {
        TIMEVAL tv, *ptv = NULL;

        if (timeout >= 0) {
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = (timeout % 1000) * 1000;
            ptv = &tv;
        }

        r = select(max_fd + 1, &rd_fds, &wr_fds, &err_fds, ptv);
        if (r == -1) {
            r = sock_error(ctxt, "select");
            goto end;
        }

        if (r) {
            OX_SockEntry *nse;

            ox_hash_foreach_safe_c(&sm->sock_hash, i, se, nse, OX_SockEntry, he) {
                int fd = OX_PTR2SIZE(se->he.key);
                int flags = 0;

                if (FD_ISSET(fd, &rd_fds))
                    flags |= OX_SOCK_EVENT_IN;
                if (FD_ISSET(fd, &wr_fds))
                    flags |= OX_SOCK_EVENT_OUT;
                if (FD_ISSET(fd, &err_fds))
                    flags |= OX_SOCK_EVENT_ERR;

                if (flags) {
                    ox_value_copy(ctxt, sock, &se->sockv);
                    ox_value_set_number(ctxt, event, flags);

                    r = ox_call(ctxt, &sm->evt_cb, ox_value_null(ctxt), sock, 2, cbr);
                    if (r == OX_ERR)
                        goto end;
                }
            }
        }
    } else {
        r = 0;
    }
#endif /*ARCH_WIN*/

#ifdef ARCH_LINUX
    r = epoll_wait(sm->epoll_fd, evts, OX_N_ELEM(evts), timeout);
    if (r == -1) {
        r = std_system_error(ctxt, "epoll_wait");
        goto end;
    }

    for (i = 0; i < r; i ++) {
        struct epoll_event *evt = &evts[i];
        int flags = 0;
        OX_SockEntry *se;

        se = ox_hash_lookup_c(ctxt, &sm->sock_hash, OX_SIZE2PTR(evt->data.fd), NULL, OX_SockEntry, he);
        if (!se)
            continue;

        ox_value_copy(ctxt, sock, &se->sockv);

        if (evt->events & EPOLLIN)
            flags |= OX_SOCK_EVENT_IN;
        if (evt->events & EPOLLOUT)
            flags |= OX_SOCK_EVENT_OUT;
        if (evt->events & (EPOLLERR|EPOLLRDHUP|EPOLLHUP))
            flags |= OX_SOCK_EVENT_ERR;

        ox_value_set_number(ctxt, event, flags);

        r = ox_call(ctxt, &sm->evt_cb, ox_value_null(ctxt), sock, 2, cbr);
        if (r == OX_ERR)
            goto end;
    }
#endif /*ARCH_LINUX*/

    ox_value_set_number(ctxt, rv, r);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, sock)
    return r;
}

/*Load this module.*/
OX_Result
ox_load (OX_Context *ctxt, OX_Value *s)
{
#ifdef ARCH_WIN
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif /*ARCH_WIN*/

    ox_not_error(ox_script_set_desc(ctxt, s, &script_desc));
    return OX_OK;
}

/*?
 *? @lib Socket.
 *?
 *? @callback SockManEventCallback Socket manager events callback function.
 *? @param sock {Socket} The socket generate events.
 *? @param events {SockMan.Event} The event type.
 *?
 *? @class{ Socket Socket.
 *?
 *? @const AF_INET Protocol family: IPV4.
 *? @const AF_INET6 Protocol family: IPV6.
 *?
 *? @const SOCK_STREAM Socket type: Sequenced, reliable, two-way, connection-based byte.
 *? @const SOCK_DGRAM Socket type: Datagrams.
 *? @const SOCK_RAW Socket type: Raw network protocol access.
 *? @const SOCK_RDM Socket type: Reliable datagram layer that does not guarantee ordering.
 *? @const SOCK_SEQPACKET Socket type: Provides a sequenced, reliable, two-way connection-based data\
 *? transmission path for datagrams of fixed maximum length; a consumer is required\
 *? to read an entire packet with each input system call.
 *?
 *? @const MSG_OOB send/recv flags: Process out-of-band data
 *? @const MSG_PEEK send/recv flags: Peek at incoming messages.
 *? @const MSG_DONTROUTE send/recv flags: Don't use local routing.
 *? @const MSG_WAITALL send/recv flags: Wait for a full request.
 *? @const MSG_NOSIGNAL send flags: No not send SIGPIPE when peer closed.
 *? @const MSG_DONTWAIT send/recv flags: Enables nonblocking operation.
 *?
 *? @func $init Initialize the socket object.
 *? @param domain {Number} The socket's protocal family.
 *? @param type {Number} The socket's type.
 *? @param protocal {Number} The protocal.
 *? @throw {SystemError} Create the socket failed.
 *?
 *? @func connect Initiate a connection on a socket
 *? @param addr {SockAddr} The connection's address.
 *? If the socket sockfd is of type SOCK_DGRAM, then addr is the address to which
 *? datagrams are sent by default, and the only address from which datagrams are
 *? received. If the socket is of type SOCK_STREAM or SOCK_SEQPACKET, this call
 *? attempts to make a connection to the socket that is bound to the address specified
 *? by addr.
 *? @throw {SystemError} Connect failed.
 *?
 *? @func bind Bind an address to the socket.
 *? @param addr {SockAddr} The address of the socket.
 *? @throw {SystemError} Bind failed.
 *?
 *? @func accept Accept a connection on the socket.
 *? @param addr {SockAddr} Return the address of the accpeted socket.
 *? @return {Socket} The accepted socket.
 *?
 *? @func listen Listen for connections on the socket.
 *? @param backlog {Number} The maximum length to which the queue of pending connections for socket may grow.
 *?
 *? @func recv Receive data from the socket.
 *? @param buf {C:void*} The buffer to store the received data.
 *? @param flags {Number} =0 The receive operation flags.
 *? @param start {Number} =0 Start position to store the received data in buf.
 *? @param len {Number} =buf.length-start Expected items' length to be read.
 *? @return {Number} Real items' length read.
 *? @throw {SystemError} Receive failed.
 *?
 *? @func send Send data to the socket.
 *? @param buf {C:void*|String} A buffer or a string contains the data to be sent.
 *? @param flags (Number) =0 The send operation flags.
 *? @param start {Number} =0 Start position to be sent in buf.
 *? @param len {Number} =buf.length-start Expected items' length to be sent.
 *? @return {Number} Real items' length sent.
 *? @throw {SystemError} Send failed.
 *?
 *? @func recvfrom Receive data from the socket with address.
 *? @param buf {C:void*} The buffer to store the received data.
 *? @param flags {Number} =0 The receive operation flags.
 *? @param addr {SockAddr} Return the source address of the data.
 *? @param start {Number} =0 Start position to store the received data in buf.
 *? @param len {Number} =buf.length-start Expected items' length to be read.
 *? @return {Number} Real items' length read.
 *? @throw {SystemError} Receive failed.
 *?
 *? @func sendto Send data to the socket with address.
 *? @param buf {C:void*|String} A buffer or a string contains the data to be sent.
 *? @param flags (Number) =0 The send operation flags.
 *? @param addr {SockAddr} The destination address.
 *? @param start {Number} =0 Start position to be sent in buf.
 *? @param len {Number} =buf.length-start Expected items' length to be sent.
 *? @return {Number} Real items' length sent.
 *? @throw {SystemError} Send failed.
 *?
 *? @func $close Close the socket.
 *?
 *? @func set_recv_timeout Set the socket's receive timeout time.
 *? @param ms {Number} Timeout time in milliseconds.
 *?
 *? @func set_send_timeout Set the socket's send timeout time.
 *? @param ms {Number} Timeout time in milliseconds.
 *?
 *? @class}
 *?
 *? @class{ SockAddr Address of the socket.
 *?
 *? @sfunc from_c Create a SockAddr object from C "struct sockaddr*".
 *? @param cptr {C:struct sockaddr*} C "struct sockaddr*".
 *? @return The new SockAddr object.
 *?
 *? @sfunc lookup Lookup the socket addreass by name.
 *? @param name {String} The name of the server.
 *? @param family {?Number} =Socket.AF_INET The protocal family.
 *? @return {[SockAddr]} Return the server's addresses array.
 *?
 *? @func $init Initialize the socket address object.
 *? @param family {Number} The protocol faimliy.
 *?
 *? @roacc family {Number} The protocol family of the address.
 *? @roacc length {Number} The length of the address in bytes.
 *? @acc port {Number} The port number of the address.
 *? @acc addr {String} The string describe the address of the socket.
 *?
 *? @class}
 *?
 *? @class{ SockMan Sockets manager.
 *?
 *? @func $init Initialize a new sockets manager.
 *? @param cb {SockManProcessCallback} Events callback function.
 *?
 *? @func add Add a new socket to the manager.
 *? @param sock {Socket} The socket to be added.
 *? @param events {SockMan.Event} The events type of the socket.
 *?
 *? @func remove Remove a socket from the manager.
 *? @param sock {Socket} The socket to be removed.
 *?
 *? @func process Process The sockets in the manager.
 *? @param timeout_ms {Number} Waiting timeout in milliseconds.
 *?
 *? @class}
 */

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_4(ctxt, c, inf, cinf, v)

    /*Socket.*/
    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "Socket"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_Socket, c));
    ox_not_error(ox_script_set_value(ctxt, s, ID_Socket_inf, inf));
    /*Socket.AF_INET*/
    ox_value_set_number(ctxt, v, AF_INET);
    ox_not_error(ox_object_add_const_s(ctxt, c, "AF_INET", v));
    /*Socket.AF_INET6*/
    ox_value_set_number(ctxt, v, AF_INET6);
    ox_not_error(ox_object_add_const_s(ctxt, c, "AF_INET6", v));
    /*Socket.SOCK_STREAM*/
    ox_value_set_number(ctxt, v, SOCK_STREAM);
    ox_not_error(ox_object_add_const_s(ctxt, c, "SOCK_STREAM", v));
    /*Socket.SOCK_DGRAM*/
    ox_value_set_number(ctxt, v, SOCK_DGRAM);
    ox_not_error(ox_object_add_const_s(ctxt, c, "SOCK_DGRAM", v));
    /*Socket.SOCK_RAW*/
    ox_value_set_number(ctxt, v, SOCK_RAW);
    ox_not_error(ox_object_add_const_s(ctxt, c, "SOCK_RAW", v));
    /*Socket.SOCK_RDM*/
    ox_value_set_number(ctxt, v, SOCK_RDM);
    ox_not_error(ox_object_add_const_s(ctxt, c, "SOCK_RDM", v));
    /*Socket.SOCK_SEQPACKET*/
    ox_value_set_number(ctxt, v, SOCK_SEQPACKET);
    ox_not_error(ox_object_add_const_s(ctxt, c, "SOCK_SEQPACKET", v));
    /*Socket.MSG_OOB*/
    ox_value_set_number(ctxt, v, MSG_OOB);
    ox_not_error(ox_object_add_const_s(ctxt, c, "MSG_OOB", v));
    /*Socket.MSG_PEEK*/
    ox_value_set_number(ctxt, v, MSG_PEEK);
    ox_not_error(ox_object_add_const_s(ctxt, c, "MSG_PEEK", v));
    /*Socket.MSG_DONTROUTE*/
    ox_value_set_number(ctxt, v, MSG_DONTROUTE);
    ox_not_error(ox_object_add_const_s(ctxt, c, "MSG_DONTROUTE", v));
    /*Socket.MSG_WAITALL*/
    ox_value_set_number(ctxt, v, MSG_WAITALL);
    ox_not_error(ox_object_add_const_s(ctxt, c, "MSG_WAITALL", v));
    /*Socket.MSG_NOSIGNAL*/
    ox_value_set_number(ctxt, v, MSG_NOSIGNAL);
    ox_not_error(ox_object_add_const_s(ctxt, c, "MSG_NOSIGNAL", v));
    /*Socket.MSG_DONTWAIT*/
    ox_value_set_number(ctxt, v, MSG_DONTWAIT);
    ox_not_error(ox_object_add_const_s(ctxt, c, "MSG_DONTWAIT", v));

    /*Socket.$inf*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$init", Socket_inf_init));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "connect", Socket_inf_connect));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "bind", Socket_inf_bind));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "accept", Socket_inf_accept));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "listen", Socket_inf_listen));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "recv", Socket_inf_recv));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "send", Socket_inf_send));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "recvfrom", Socket_inf_recvfrom));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "sendto", Socket_inf_sendto));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$close", Socket_inf_close));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "set_recv_timeout", Socket_inf_set_recv_timeout));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "set_send_timeout", Socket_inf_set_send_timeout));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "add_membership", Socket_inf_add_membership));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "set_nonblock", Socket_inf_set_nonblock));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "fd", Socket_inf_fd_get, NULL));

    /*SockAddr.*/
    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "SockAddr"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_SockAddr, c));
    ox_not_error(ox_script_set_value(ctxt, s, ID_SockAddr_inf, inf));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "from_c", SockAddr_from_c));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "lookup", SockAddr_lookup));
    /*SockAddr.INADDR_ANY*/
    ox_value_set_number(ctxt, v, INADDR_ANY);
    ox_not_error(ox_object_add_const_s(ctxt, c, "INADDR_ANY", v));

    /*SockAddr.$inf*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$init", SockAddr_inf_init));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "to_c", SockAddr_inf_to_c));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "length", SockAddr_inf_length_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "family", SockAddr_inf_family_get, NULL));

    /*SockAddrInet_inf*/
    ox_not_error(ox_interface_new(ctxt, cinf));
    ox_not_error(ox_interface_inherit(ctxt, cinf, inf));
    ox_not_error(ox_script_set_value(ctxt, s, ID_SockAddrInet_inf, cinf));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, cinf, "port", SockAddrInet_inf_port_get, SockAddrInet_inf_port_set));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, cinf, "addr", SockAddrInet_inf_addr_get, SockAddrInet_inf_addr_set));

    /*SockAddrInet6_inf*/
    ox_not_error(ox_interface_new(ctxt, cinf));
    ox_not_error(ox_interface_inherit(ctxt, cinf, inf));
    ox_not_error(ox_script_set_value(ctxt, s, ID_SockAddrInet6_inf, cinf));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, cinf, "port", SockAddrInet6_inf_port_get, SockAddrInet6_inf_port_set));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, cinf, "addr", SockAddrInet6_inf_addr_get, SockAddrInet6_inf_addr_set));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, cinf, "flowinfo", SockAddrInet6_inf_flowinfo_get, SockAddrInet6_inf_flowinfo_set));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, cinf, "scope_id", SockAddrInet6_inf_scope_id_get, SockAddrInet6_inf_scope_id_set));

    /*sockaddr*/
    ox_not_error(ox_ctype_struct(ctxt, c, NULL, sizeof(struct sockaddr)));
    ox_not_error(ox_script_set_value(ctxt, s, ID_sockaddr, c));

    /*SockMan*/
    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "SockMan"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_SockMan, c));
    ox_not_error(ox_enum_new(ctxt, v, OX_ENUM_BITFIELD));
    ox_not_error(ox_object_add_const_s(ctxt, c, "Event", v));
    ox_not_error(ox_enum_add_item_s(ctxt, v, c, "EVENT_IN", OX_SOCK_EVENT_IN));
    ox_not_error(ox_enum_add_item_s(ctxt, v, c, "EVENT_OUT", OX_SOCK_EVENT_OUT));
    ox_not_error(ox_enum_add_item_s(ctxt, v, c, "EVENT_ERR", OX_SOCK_EVENT_ERR));

    /*SockMan.$inf*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$init", SockMan_inf_init));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "add", SockMan_inf_add));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "remove", SockMan_inf_remove));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "process", SockMan_inf_process));

    OX_VS_POP(ctxt, c)
    return OX_OK;
}
