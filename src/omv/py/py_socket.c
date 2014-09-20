/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Socket Python module.
 *
 */
#include <mp.h>
#include <cc3000_common.h>
#include <evnt_handler.h>
#include <socket.h>
#include <inet_pton.h>
#include <inet_ntop.h>
#include <py_wlan.h>
#include <py_socket.h>
#include <py_assert.h>

#define MAX_FD              (8)
#define MAX_ADDRSTRLEN      (128)
#define MAX_RX_PACKET       (CC3000_RX_BUFFER_SIZE-CC3000_MINIMAL_RX_SIZE-1)
#define MAX_TX_PACKET       (CC3000_TX_BUFFER_SIZE-CC3000_MINIMAL_TX_SIZE-1)
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

const mp_obj_type_t socket_type;

void socket_print(void (*print)(void *env, const char *fmt, ...), void *env, mp_obj_t self_in, mp_print_kind_t kind)
{
    printf("<%s %p>", mp_obj_get_type_str(self_in), self_in);
}

static mp_uint_t socket_send(mp_obj_t self_in, const void *buf, mp_uint_t size, int *errcode)
{
    int bytes = 0;
    socket_t *self = self_in;

    if (wlan_get_fd_state(self->fd)) {
        closesocket(self->fd);
        *errcode = 32;
        return 0;
    }

    /* send packets */
    while (bytes < size) {
        int n = min((size-bytes), MAX_TX_PACKET);
        n = send(self->fd, buf+bytes, n, 0);
        if (n <= 0) {
            bytes = n;
            *errcode = errno;
            break;
        }
        bytes += n;
    }

    return bytes;
}

static mp_uint_t socket_recv(mp_obj_t self_in, void *buf, mp_uint_t size, int *errcode)
{
    int bytes = 0;
    socket_t *self = self_in;

    if (wlan_get_fd_state(self->fd)) {
        closesocket(self->fd);
        *errcode = 0;
        return 0;
    }

    /* recv packets */
    while (bytes < size) {
        int n = min((size-bytes), MAX_RX_PACKET);
        n = recv(self->fd, buf+bytes, n, 0);
        if (n == 0) {
            break;
        } else if (n < 0) {
            bytes = n;
            *errcode = errno;
            break;
        }
        bytes += n;
    }

    return bytes;
}

static mp_obj_t socket_bind(mp_obj_t self_in, mp_obj_t addr_obj)
{
    socket_t *self = self_in;

    mp_obj_t *addr;
    mp_obj_get_array_fixed_n(addr_obj, 2, &addr);

    /* fill sockaddr */
    int port = mp_obj_get_int(addr[1]);
    sockaddr_in addr_in = {
        .sin_family = AF_INET,
        .sin_port   = htons(port),
        .sin_addr.s_addr = 0,// INADDR_ANY
        .sin_zero   = {0}
    };

    const char *host = mp_obj_str_get_str(addr[0]);
    if (strlen(host) && !inet_pton(AF_INET, host, &addr_in.sin_addr.s_addr)) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "invalid IP address"));
    }

    /* bind socket */
    if (bind(self->fd, (sockaddr*) &addr_in, sizeof(sockaddr_in)) < 0) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "bind failed"));
    }
    return mp_const_true;
}

static mp_obj_t socket_listen(mp_obj_t self_in, mp_obj_t backlog)
{
    socket_t *self = self_in;
    if (listen(self->fd, mp_obj_get_int(backlog)) < 0) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "listen failed"));
    }

    return mp_const_true;
}

static mp_obj_t socket_accept(mp_obj_t self_in)
{
    int fd;
    socket_t *self = self_in;

    sockaddr addr;
    socklen_t addr_len = sizeof(sockaddr);

    // accept incoming connection
    if ((fd = accept(self->fd, &addr, &addr_len)) < 0) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "accept failed"));
    }

    // clear socket state
    wlan_clear_fd_state(fd);

    // create new socket object
    socket_t *socket_obj = m_new_obj_with_finaliser(socket_t);
    socket_obj->base.type = (mp_obj_t)&socket_type;
    socket_obj->fd  = fd;


    char buf[MAX_ADDRSTRLEN]={0};
    if (inet_ntop(addr.sa_family,
        &(((sockaddr_in*)&addr)->sin_addr), buf, MAX_ADDRSTRLEN) == NULL) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "invalid IP address"));
    }

    mp_obj_tuple_t *cli = mp_obj_new_tuple(2, NULL);
    mp_obj_tuple_t *cli_addr = mp_obj_new_tuple(2, NULL);

    cli->items[0] = socket_obj;
    cli->items[1] = cli_addr;
    cli_addr->items[0] = mp_obj_new_str(buf, strlen(buf), false);
    cli_addr->items[1] = mp_obj_new_int(((sockaddr_in*)&addr)->sin_port);

    return cli;
}

static mp_obj_t socket_settimeout(mp_obj_t self_in, mp_obj_t timeout)
{
    socket_t *self = self_in;
    int optval = mp_obj_get_int(timeout);
    socklen_t optlen = sizeof(optval);

    if (setsockopt(self->fd, SOL_SOCKET, SOCKOPT_RECV_TIMEOUT, &optval, optlen) != 0) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "setsockopt failed"));
    }

    return mp_const_true;
}

static mp_obj_t socket_setblocking(mp_obj_t self_in, mp_obj_t blocking)
{
    socket_t *self = self_in;
    int optval;
    socklen_t optlen = sizeof(optval);

    if (mp_obj_get_int(blocking)) {
        optval = SOCK_OFF; // Enable non-blocking
    } else {
        optval = SOCK_ON;
    }

    if (setsockopt(self->fd, SOL_SOCKET, SOCKOPT_RECV_NONBLOCK, &optval, optlen) != 0 ||
        setsockopt(self->fd, SOL_SOCKET, SOCKOPT_ACCEPT_NONBLOCK, &optval, optlen) != 0 ) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "setsockopt failed"));
    }

    return mp_const_true;
}

static mp_obj_t socket_close(mp_obj_t self_in)
{
    closesocket(((socket_t *)self_in)->fd);
    return mp_const_none;
}

static mp_obj_t socket_new(mp_obj_t domain, mp_obj_t type, mp_obj_t protocol)
{
    socket_t *socket_obj = m_new_obj_with_finaliser(socket_t);
    socket_obj->base.type = (mp_obj_t)&socket_type;

    /* create new socket */
    socket_obj->fd = socket(mp_obj_get_int(domain), mp_obj_get_int(type), mp_obj_get_int(protocol));
    if (socket_obj->fd < 0) {
        m_del_obj(socket_t, socket_obj);
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "socket failed"));
    }

    // clear socket state
    wlan_clear_fd_state(socket_obj->fd);
    return socket_obj;
}

static MP_DEFINE_CONST_FUN_OBJ_2(socket_bind_obj,       socket_bind);
static MP_DEFINE_CONST_FUN_OBJ_2(socket_listen_obj,     socket_listen);
static MP_DEFINE_CONST_FUN_OBJ_1(socket_accept_obj,     socket_accept);
static MP_DEFINE_CONST_FUN_OBJ_2(socket_settimeout_obj, socket_settimeout);
static MP_DEFINE_CONST_FUN_OBJ_2(socket_setblocking_obj,socket_setblocking);
static MP_DEFINE_CONST_FUN_OBJ_1(socket_close_obj,      socket_close);
static MP_DEFINE_CONST_FUN_OBJ_3(socket_new_obj,        socket_new);

static const mp_map_elem_t socket_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_send),        (mp_obj_t)&mp_stream_write_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_recv),        (mp_obj_t)&mp_stream_read_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_bind),        (mp_obj_t)&socket_bind_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_listen),      (mp_obj_t)&socket_listen_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_accept),      (mp_obj_t)&socket_accept_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_settimeout),  (mp_obj_t)&socket_settimeout_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_setblocking), (mp_obj_t)&socket_setblocking_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_close),       (mp_obj_t)&socket_close_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR___del__),     (mp_obj_t)&socket_close_obj },
};

static MP_DEFINE_CONST_DICT(socket_locals_dict, socket_locals_dict_table);

static const mp_stream_p_t socket_stream_p = {
    .read = socket_recv,
    .write = socket_send,
};

const mp_obj_type_t socket_type = {
    { &mp_type_type },
    .name = MP_QSTR_socket,
    .print = socket_print,
    .getiter = NULL,
    .iternext = NULL,
    .stream_p = &socket_stream_p,
    .locals_dict = (mp_obj_t)&socket_locals_dict,
};

/* Socket module */
static const mp_map_elem_t module_globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),        MP_OBJ_NEW_QSTR(MP_QSTR_socket) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_socket),          (mp_obj_t)&socket_new_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_AF_INET),         MP_OBJ_NEW_SMALL_INT(AF_INET)  },
    { MP_OBJ_NEW_QSTR(MP_QSTR_AF_INET6),        MP_OBJ_NEW_SMALL_INT(AF_INET6) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_SOCK_STREAM),     MP_OBJ_NEW_SMALL_INT(SOCK_STREAM) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_SOCK_DGRAM),      MP_OBJ_NEW_SMALL_INT(SOCK_DGRAM) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_SOCK_RAW),        MP_OBJ_NEW_SMALL_INT(SOCK_RAW) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_IPPROTO_IP)  ,    MP_OBJ_NEW_SMALL_INT(IPPROTO_IP)  },
    { MP_OBJ_NEW_QSTR(MP_QSTR_IPPROTO_ICMP),    MP_OBJ_NEW_SMALL_INT(IPPROTO_ICMP) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_IPPROTO_IPV4),    MP_OBJ_NEW_SMALL_INT(IPPROTO_IPV4) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_IPPROTO_TCP),     MP_OBJ_NEW_SMALL_INT(IPPROTO_TCP) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_IPPROTO_UDP),     MP_OBJ_NEW_SMALL_INT(IPPROTO_UDP) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_IPPROTO_IPV6),    MP_OBJ_NEW_SMALL_INT(IPPROTO_IPV6) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_IPPROTO_RAW),     MP_OBJ_NEW_SMALL_INT(IPPROTO_RAW) },
};

static MP_DEFINE_CONST_DICT(module_globals_dict, module_globals_dict_table);

const mp_obj_module_t socket_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_socket,
    .globals = (mp_obj_dict_t*)&module_globals_dict,
};

const mp_obj_module_t *py_socket_init()
{
    return &socket_module;
}
