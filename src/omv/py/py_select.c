#include <mp.h>
#include <cc3000_common.h>
#include <evnt_handler.h>
#include <socket.h>
#include <inet_pton.h>
#include <inet_ntop.h>
#include <py_wlan.h>
#include <py_socket.h>
#include <py_select.h>
#include <py_assert.h>

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

// select helper functions
static void set_fds(int *nfds, mp_obj_t *fdlist, uint fdlist_len, fd_set *fdset)
{
    FD_ZERO(fdset);

    /* add sockets to fd set*/
    for (int i=0; i<fdlist_len; i++) {
        socket_t *s = fdlist[i];

        /* check arg type*/
        PY_ASSERT_TYPE(s, &socket_type);

        /* add to fd set */
        FD_SET(s->fd, fdset);

        if (s->fd > (*nfds)) {
            *nfds = s->fd;
        }
    }
}

static void get_fds(mp_obj_t *fdlist, uint fdlist_len, mp_obj_t *fdlist_out, fd_set *fdset)
{
    for (int i=0; i<fdlist_len; i++) {
        socket_t *s = fdlist[i];
        if (FD_ISSET(s->fd, fdset)) {
            socket_t *socket_obj = m_new_obj_with_finaliser(socket_t);
            socket_obj->base.type = (mp_obj_t)&socket_type;
            socket_obj->fd  = s->fd;
            mp_obj_list_append(fdlist_out, socket_obj);
        }
    }
}

static mp_obj_t py_select(uint n_args, const mp_obj_t *args)
{
    int nfds=0; //highest-numbered fd plus 1
    timeval tv={0};
    fd_set rfds, wfds, xfds;

    mp_obj_t *rlist, *wlist, *xlist;
    uint rlist_len, wlist_len, xlist_len;

    /* read args */
    mp_obj_get_array(args[0], &rlist_len, &rlist);
    mp_obj_get_array(args[1], &wlist_len, &wlist);
    mp_obj_get_array(args[2], &xlist_len, &xlist);

    if (n_args == 4) {
        float timeout = mp_obj_get_float(args[3]);
        tv.tv_sec = (int)timeout;
        tv.tv_usec = (timeout-(int)timeout)*1000*1000;
    }

    // add fds to their respective sets
    set_fds(&nfds, rlist, rlist_len, &rfds);
    set_fds(&nfds, wlist, wlist_len, &wfds);
    set_fds(&nfds, xlist, xlist_len, &xfds);

    // call select
    nfds = select(nfds+1, &rfds, &wfds, &xfds, &tv);

    // if any of the read sockets is closed, we add it to the read fd set,
    // a subsequent call to recv() returns 0. This behavior is consistent with BSD.
    for (int i=0; i<rlist_len; i++) {
        socket_t *s = rlist[i];
        if (wlan_get_fd_state(s->fd)) {
            FD_SET(s->fd, &rfds);
            nfds = max(nfds, s->fd);
        }
    }

    // return value; a tuple of 3 lists
    mp_obj_t fds[3] = {
        mp_obj_new_list(0, NULL),
        mp_obj_new_list(0, NULL),
        mp_obj_new_list(0, NULL)
    };

    // On success, select() returns the number of file descriptors contained
    // in the three returned descriptor sets which may be zero if the timeout
    // expires before anything interesting happens, -1 is returned on error.
    if (nfds == -1) {   // select failed
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "select failed"));
    } else if (nfds) {  // an fd is ready
        get_fds(rlist, rlist_len, fds[0], &rfds);
        get_fds(wlist, wlist_len, fds[1], &wfds);
        get_fds(xlist, xlist_len, fds[2], &xfds);
    } // select timedout

    return mp_obj_new_tuple(3, fds);
}

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_select_obj, 3, 4, py_select);
// select module
static const mp_map_elem_t module_globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),    MP_OBJ_NEW_QSTR(MP_QSTR_select) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_select),      (mp_obj_t)&py_select_obj },
};

static MP_DEFINE_CONST_DICT(module_globals_dict, module_globals_dict_table);

const mp_obj_module_t select_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_select,
    .globals = (mp_obj_dict_t*)&module_globals_dict,
};

const mp_obj_module_t *py_select_init()
{
    return &select_module;
}



