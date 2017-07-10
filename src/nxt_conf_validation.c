
/*
 * Copyright (C) Valentin V. Bartenev
 * Copyright (C) NGINX, Inc.
 */

#include <nxt_main.h>
#include <nxt_conf.h>


typedef struct {
    nxt_str_t   name;
    nxt_uint_t  type;
    nxt_int_t   (*validator)(nxt_conf_value_t *conf, nxt_conf_value_t *value,
                             void *data);
    void        *data;
} nxt_conf_vldt_object_t;


typedef nxt_int_t (*nxt_conf_vldt_member_t)(nxt_conf_value_t *conf,
                                            nxt_str_t *name,
                                            nxt_conf_value_t *value);

static nxt_int_t nxt_conf_vldt_listener(nxt_conf_value_t *conf, nxt_str_t *name,
    nxt_conf_value_t *value);
static nxt_int_t nxt_conf_vldt_app_name(nxt_conf_value_t *conf,
    nxt_conf_value_t *value, void *data);
static nxt_int_t nxt_conf_vldt_app(nxt_conf_value_t *conf, nxt_str_t *name,
    nxt_conf_value_t *value);
static nxt_int_t nxt_conf_vldt_object(nxt_conf_value_t *conf,
    nxt_conf_value_t *value, void *data);
static nxt_int_t nxt_conf_vldt_object_iterator(nxt_conf_value_t *conf,
    nxt_conf_value_t *value, void *data);


static nxt_conf_vldt_object_t  nxt_conf_vldt_root_members[] = {
    { nxt_string("listeners"),
      NXT_CONF_OBJECT,
      &nxt_conf_vldt_object_iterator,
      &nxt_conf_vldt_listener },

    { nxt_string("applications"),
      NXT_CONF_OBJECT,
      &nxt_conf_vldt_object_iterator,
      &nxt_conf_vldt_app },

    { nxt_null_string, 0, NULL, NULL }
};


static nxt_conf_vldt_object_t  nxt_conf_vldt_listener_members[] = {
    { nxt_string("application"),
      NXT_CONF_STRING,
      &nxt_conf_vldt_app_name,
      NULL },

    { nxt_null_string, 0, NULL, NULL }
};


static nxt_conf_vldt_object_t  nxt_conf_vldt_python_members[] = {
    { nxt_string("type"),
      NXT_CONF_STRING,
      NULL,
      NULL },

    { nxt_string("workers"),
      NXT_CONF_INTEGER,
      NULL,
      NULL },

    { nxt_string("user"),
      NXT_CONF_STRING,
      NULL,
      NULL },

    { nxt_string("group"),
      NXT_CONF_STRING,
      NULL,
      NULL },

    { nxt_string("path"),
      NXT_CONF_STRING,
      NULL,
      NULL },

    { nxt_string("module"),
      NXT_CONF_STRING,
      NULL,
      NULL },

    { nxt_null_string, 0, NULL, NULL }
};


static nxt_conf_vldt_object_t  nxt_conf_vldt_php_members[] = {
    { nxt_string("type"),
      NXT_CONF_STRING,
      NULL,
      NULL },

    { nxt_string("workers"),
      NXT_CONF_INTEGER,
      NULL,
      NULL },

    { nxt_string("user"),
      NXT_CONF_STRING,
      NULL,
      NULL },

    { nxt_string("group"),
      NXT_CONF_STRING,
      NULL,
      NULL },

    { nxt_string("root"),
      NXT_CONF_STRING,
      NULL,
      NULL },

    { nxt_string("script"),
      NXT_CONF_STRING,
      NULL,
      NULL },

    { nxt_string("index"),
      NXT_CONF_STRING,
      NULL,
      NULL },

    { nxt_null_string, 0, NULL, NULL }
};


static nxt_conf_vldt_object_t  nxt_conf_vldt_go_members[] = {
    { nxt_string("type"),
      NXT_CONF_STRING,
      NULL,
      NULL },

    { nxt_string("workers"),
      NXT_CONF_INTEGER,
      NULL,
      NULL },

    { nxt_string("user"),
      NXT_CONF_STRING,
      NULL,
      NULL },

    { nxt_string("group"),
      NXT_CONF_STRING,
      NULL,
      NULL },

    { nxt_string("executable"),
      NXT_CONF_STRING,
      NULL,
      NULL },

    { nxt_null_string, 0, NULL, NULL }
};


nxt_int_t
nxt_conf_validate(nxt_conf_value_t *value)
{
    if (nxt_conf_type(value) != NXT_CONF_OBJECT) {
        return NXT_ERROR;
    }

    return nxt_conf_vldt_object(value, value, nxt_conf_vldt_root_members);
}


static nxt_int_t
nxt_conf_vldt_listener(nxt_conf_value_t *conf, nxt_str_t *name,
    nxt_conf_value_t *value)
{
    return nxt_conf_vldt_object(conf, value, nxt_conf_vldt_listener_members);
}


static nxt_int_t
nxt_conf_vldt_app_name(nxt_conf_value_t *conf, nxt_conf_value_t *value,
    void *data)
{
    nxt_str_t         name;
    nxt_conf_value_t  *apps, *app;

    static nxt_str_t  apps_str = nxt_string("applications");

    apps = nxt_conf_get_object_member(conf, &apps_str, NULL);

    if (nxt_slow_path(apps == NULL)) {
        return NXT_ERROR;
    }

    nxt_conf_get_string(value, &name);

    app = nxt_conf_get_object_member(apps, &name, NULL);

    if (nxt_slow_path(app == NULL)) {
        return NXT_ERROR;
    }

    return NXT_OK;
}


static nxt_int_t
nxt_conf_vldt_app(nxt_conf_value_t *conf, nxt_str_t *name,
    nxt_conf_value_t *value)
{
    nxt_str_t         type;
    nxt_conf_value_t  *type_value;

    static nxt_str_t  type_str = nxt_string("type");

    type_value = nxt_conf_get_object_member(value, &type_str, NULL);

    if (nxt_slow_path(type_value == NULL)) {
        return NXT_ERROR;
    }

    if (nxt_conf_type(type_value) != NXT_CONF_STRING) {
        return NXT_ERROR;
    }

    nxt_conf_get_string(type_value, &type);

    if (nxt_str_eq(&type, "python", 6)) {
        return nxt_conf_vldt_object(conf, value, nxt_conf_vldt_python_members);
    }

    if (nxt_str_eq(&type, "php", 3)) {
        return nxt_conf_vldt_object(conf, value, nxt_conf_vldt_php_members);
    }

    if (nxt_str_eq(&type, "go", 2)) {
        return nxt_conf_vldt_object(conf, value, nxt_conf_vldt_go_members);
    }

    return NXT_ERROR;
}


static nxt_int_t
nxt_conf_vldt_object(nxt_conf_value_t *conf, nxt_conf_value_t *value,
    void *data)
{
    uint32_t                index;
    nxt_str_t               name;
    nxt_conf_value_t        *member;
    nxt_conf_vldt_object_t  *vldt;

    index = 0;

    for ( ;; ) {
        member = nxt_conf_next_object_member(value, &name, &index);

        if (member == NULL) {
            return NXT_OK;
        }

        vldt = data;

        for ( ;; ) {
            if (vldt->name.length == 0) {
                return NXT_ERROR;
            }

            if (!nxt_strstr_eq(&vldt->name, &name)) {
                vldt++;
                continue;
            }

            if (nxt_conf_type(member) != vldt->type) {
                return NXT_ERROR;
            }

            if (vldt->validator != NULL
                && vldt->validator(conf, member, vldt->data) != NXT_OK)
            {
                return NXT_ERROR;
            }

            break;
        }
    }
}


static nxt_int_t
nxt_conf_vldt_object_iterator(nxt_conf_value_t *conf, nxt_conf_value_t *value,
    void *data)
{
    uint32_t                index;
    nxt_str_t               name;
    nxt_conf_value_t        *member;
    nxt_conf_vldt_member_t  validator;

    validator = data;
    index = 0;

    for ( ;; ) {
        member = nxt_conf_next_object_member(value, &name, &index);

        if (member == NULL) {
            return NXT_OK;
        }

        if (validator(conf, &name, member) != NXT_OK) {
            return NXT_ERROR;
        }
    }

    return NXT_OK;
}
