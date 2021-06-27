#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "Zend/zend_smart_str.h" /* for smart_str */
#include "Zend/zend_interfaces.h" /* for zend_class_serialize_deny */
#include "ext/standard/php_string.h"
#include "ext/pcre/php_pcre.h"

#include "php_lxx.h"
#include "lxx_router.h"
#include "tools/lxx_radix_tree.h"
#include "lxx_request.h"

zend_class_entry *lxx_router_ce;
static zend_object_handlers lxx_router_handlers;

static zend_object *lxx_router_new(zend_class_entry *ce) {
    lxx_router_t *router = emalloc(sizeof(lxx_router_t) + zend_object_properties_size(ce));
    memset(router, 0, XtOffsetOf(lxx_router_t, std));
    router->idx = -1;

    lxx_rax_tree_instance(&router->radix_tree);
    lxx_request_instance(&router->request);

    ALLOC_HASHTABLE(router->routes);
    zend_hash_init(router->routes, 0, NULL, ZVAL_PTR_DTOR, 0);

    zend_object_std_init(&router->std, ce);
    router->std.handlers = &lxx_router_handlers;

    return &router->std;
}

static lxx_router_t *lxx_router_fetch(zend_object *object) {
    return (lxx_router_t *) ((char *) object - lxx_router_handlers.offset);
}

static void lxx_router_free(zend_object *object) {
    lxx_router_t *router = lxx_router_fetch(object);

    if (router->routes) {
		if (GC_DELREF(router->routes) == 0) {
			GC_REMOVE_FROM_BUFFER(router->routes);
			zend_array_destroy(router->routes);
		}
	}

    if (router->controller) {
        zend_string_release(router->controller);
    }

    if (router->action) {
        zend_string_release(router->action);
    }

    zval_ptr_dtor(&router->radix_tree);
    zval_ptr_dtor(&router->request);
    zend_object_std_dtor(object);
}

void lxx_router_instance(zval *this_ptr) {
    object_init_ex(this_ptr, lxx_router_ce);
}

static void lxx_router_insert_router(lxx_router_t *router, zend_string *path, zval *pData) {
    zend_long idx = lxx_rax_tree_find(Z_OBJ_P(&router->radix_tree), path);
    if (idx != -1) {
        return;
    }

    router->idx++;
    zend_hash_index_add(router->routes, router->idx, pData);
    
    lxx_rax_tree_insert(Z_OBJ(router->radix_tree), path, router->idx);
}

static zend_string *lxx_router_generating_regex(const char *path) {
    smart_str pattern = {0};
    size_t i;
    size_t l = strlen(path);
    zend_string *regex_str;

    if (!l) {
        return zend_string_init("", 0, 0);
    }

    smart_str_appendl(&pattern, "#^", sizeof("#^") - 1);
    for (i = 0; i < l; i++) {
        if (path[i] == '*') {
            smart_str_appendl(&pattern, "(?P<all>.*)", sizeof("(?P<all>.*)") -1);
            break;
        } else if (path[i] == '{') {
            smart_str_appendl(&pattern, "(?P<", sizeof("(?P<")-1);
        }else if (path[i] == '}') {
			smart_str_appendl(&pattern, ">.+?)", sizeof(">.+?)")-1);
		} else {
			smart_str_appendc(&pattern, path[i]);
		}
    }

    smart_str_appendc(&pattern, '#');
	smart_str_appendc(&pattern, 'i');
	smart_str_0(&pattern);
    regex_str = zend_string_copy(pattern.s);
    smart_str_free(&pattern);

    return regex_str;
}

static void lxx_router_add_router(INTERNAL_FUNCTION_PARAMETERS, const char *method) {
    zend_string *path;
    zend_string *new_path;
    zend_string *key;
    zend_string *reg_str;
    zval *func;
    zval pData;
    char *pos;
    char *spos;
    int mlen = strlen(method);

    lxx_router_t *router = lxx_router_fetch(Z_OBJ_P(getThis()));
    
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sz", &path, &func) == FAILURE) {
        return;
    }

    // ZVAL_COPY(&pData, func);
    new_path = zend_string_alloc(ZSTR_LEN(path) + mlen, 0);
    memcpy(ZSTR_VAL(new_path), method, mlen);
    memcpy(ZSTR_VAL(new_path) + mlen, ZSTR_VAL(path), ZSTR_LEN(path) + 1);

    pos = strchr(ZSTR_VAL(new_path), '{');// - ZSTR_VAL(new_path);
    if (pos != NULL) {
        key = zend_string_init(ZSTR_VAL(new_path), pos - ZSTR_VAL(new_path), 0);
    } else if ((spos = strchr(ZSTR_VAL(new_path), '*')) != NULL) {
        key = zend_string_init(ZSTR_VAL(new_path), spos - ZSTR_VAL(new_path), 0);
    } else {
        key = zend_string_init(ZSTR_VAL(new_path), ZSTR_LEN(new_path), 0);
    }

    reg_str = lxx_router_generating_regex(ZSTR_VAL(new_path) + ZSTR_LEN(key));

    zval t;
    array_init(&pData);

    ZVAL_STR_COPY(&t, reg_str);
    zend_hash_str_add(Z_ARR(pData), "reg", sizeof("reg") - 1, &t);

    ZVAL_STR_COPY(&t, key);
    zend_hash_str_add(Z_ARR(pData), "key", sizeof("key") - 1, &t);

    ZVAL_COPY(&t, func);
    zend_hash_str_add(Z_ARR(pData), "func", sizeof("func") - 1, &t);

    lxx_router_insert_router(router, key,  &pData);
    zend_string_release(new_path);
    zend_string_release(key);
    zend_string_release(reg_str);
}

zval *lxx_router_match_router(zend_object *object) {
    zend_string *method;
    zend_string *base_uri;
    zend_string *new_path;
    zend_long idx;
    zval *route;
    pcre_cache_entry *pce_regexp;

    lxx_router_t *router = lxx_router_fetch(object);

    method = lxx_request_get_method(Z_OBJ(router->request));
    if (!method) {
        zend_string_release(method);
        return NULL;
    }

    base_uri = lxx_request_get_base_uri(Z_OBJ(router->request));
    if (!base_uri) {
        base_uri = zend_string_init("/", sizeof("/")-1, 0);
    }

    if (ZSTR_LEN(method) <= 0 ) {
        zend_string_release(method);
        zend_string_release(base_uri);
        return NULL;
    }

    size_t len = ZSTR_LEN(method) + ZSTR_LEN(base_uri);
    new_path = zend_string_alloc(len, 0);
    memcpy(ZSTR_VAL(new_path), ZSTR_VAL(method), ZSTR_LEN(method));
    memcpy(ZSTR_VAL(new_path) + ZSTR_LEN(method), ZSTR_VAL(base_uri), ZSTR_LEN(base_uri));
    
    idx = lxx_rax_tree_pre_seach(Z_OBJ(router->radix_tree), "<=", new_path);
    zend_string_release(method);
    zend_string_release(base_uri);

    if (idx == -1) {
        zend_string_release(new_path);
        return NULL;;
    }

    route = zend_hash_index_find(router->routes, idx);

    zval *regexp = zend_hash_str_find(Z_ARR_P(route), "reg", sizeof("reg") - 1);
    zval *key = zend_hash_str_find(Z_ARR_P(route), "key", sizeof("key") - 1);
    zval *func = zend_hash_str_find(Z_ARR_P(route), "func", sizeof("func") - 1);

    if (!regexp || Z_STRLEN_P(regexp) <= 0) {
        zend_string_release(new_path);
        return func;
    }
    
    // php_var_dump(route, 2);
    char *regexp_path = ZSTR_VAL(new_path) + Z_STRLEN_P(key);
    size_t rlen = ZSTR_LEN(new_path) - Z_STRLEN_P(key);

    if (rlen <= 0) {
        zend_string_release(new_path);
        return func;
    }
    
    if ((pce_regexp = pcre_get_compiled_regex_cache(Z_STR_P(regexp))) == NULL) {
        zend_string_release(new_path);
		return func;
	}

    zval matches, subparts;
    ZVAL_NULL(&subparts);
#if PHP_VERSION_ID < 70400
    php_pcre_match_impl(pce_regexp, regexp_path, rlen, &matches, &subparts, 0, 0, 0, 0);
#else
    {
        zend_string *tmp = zend_string_init(regexp_path, rlen, 0);
        php_pcre_match_impl(pce_regexp, tmp, &matches, &subparts, 0, 0, 0, 0);
        zend_string_release(tmp);
    }
#endif

    zend_string_release(new_path);

    if (!zend_hash_num_elements(Z_ARRVAL(subparts))) {
        zval_ptr_dtor(&matches);
        zval_ptr_dtor(&subparts);
        return func;
    }
    
    lxx_request_set_params(Z_OBJ(router->request), &subparts);
    
    zval_ptr_dtor(&matches);
    zval_ptr_dtor(&subparts);
    return func;
}

void lxx_router_set_controller(zend_object *object, zend_string *controller) {
    lxx_router_t *router = lxx_router_fetch(object);
    router->controller = zend_string_copy(controller);
}

void lxx_router_set_action(zend_object *object, zend_string *action) {
    lxx_router_t *router = lxx_router_fetch(object);
    router->action = zend_string_copy(action);
}

ZEND_METHOD(lxx_router, __construct) {
    
}

ZEND_METHOD(lxx_router, get) {
    lxx_router_add_router(INTERNAL_FUNCTION_PARAM_PASSTHRU, "GET");
}

ZEND_METHOD(lxx_router, post) {
    lxx_router_add_router(INTERNAL_FUNCTION_PARAM_PASSTHRU, "POST");
}

ZEND_METHOD(lxx_router, put) {
    lxx_router_add_router(INTERNAL_FUNCTION_PARAM_PASSTHRU, "PUT");
}

ZEND_METHOD(lxx_router, delete) {
    lxx_router_add_router(INTERNAL_FUNCTION_PARAM_PASSTHRU, "DELETE");
}

ZEND_METHOD(lxx_router, patch) {
    lxx_router_add_router(INTERNAL_FUNCTION_PARAM_PASSTHRU, "PATCH");
}

ZEND_METHOD(lxx_router, head) {
    lxx_router_add_router(INTERNAL_FUNCTION_PARAM_PASSTHRU, "HEAD");
}

ZEND_METHOD(lxx_router, options) {
    lxx_router_add_router(INTERNAL_FUNCTION_PARAM_PASSTHRU, "OPTIONS");
}

ZEND_METHOD(lxx_router, connect) {
    lxx_router_add_router(INTERNAL_FUNCTION_PARAM_PASSTHRU, "CONNECT");
}

ZEND_METHOD(lxx_router, trace) {
    lxx_router_add_router(INTERNAL_FUNCTION_PARAM_PASSTHRU, "TRACE");
}

ZEND_METHOD(lxx_router, cli) {
    lxx_router_add_router(INTERNAL_FUNCTION_PARAM_PASSTHRU, "CLI");
}

ZEND_METHOD(lxx_router, match) {
    zval *func = lxx_router_match_router(Z_OBJ_P(getThis()));

    if (func) {
        zval retval;
        call_user_function(CG(function_table), NULL, func, &retval, 0, NULL);
        zval_ptr_dtor(&retval);
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

zend_function_entry lxx_router_methods[] = {
    ZEND_ME(lxx_router, __construct, NULL, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_router, get, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_router, post, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_router, put, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_router, delete, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_router, patch, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_router, head, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_router, options, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_router, connect, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_router, trace, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_router, cli, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_router, match, NULL, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

LXX_MINIT_FUNCTION(router) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Lxx\\Router", lxx_router_methods);
    lxx_router_ce = zend_register_internal_class(&ce);
    lxx_router_ce->ce_flags |= ZEND_ACC_FINAL;
    lxx_router_ce->create_object = lxx_router_new;

    memcpy(&lxx_router_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    lxx_router_handlers.offset = XtOffsetOf(lxx_router_t, std);
    lxx_router_handlers.free_obj = lxx_router_free;

    return SUCCESS;
}