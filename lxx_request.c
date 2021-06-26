#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "main/SAPI.h"
#include "Zend/zend_interfaces.h"

#include "php_lxx.h"
#include "lxx_request.h"

zend_class_entry *lxx_request_ce;
static zend_object_handlers lxx_request_handlers;

static zend_object *lxx_request_new(zend_class_entry *ce) {
    lxx_request_t *request = emalloc(sizeof(lxx_request_t) + zend_object_properties_size(ce));
    memset(request, 0, XtOffsetOf(lxx_request_t, std));

    ALLOC_HASHTABLE(request->params);
    zend_hash_init(request->params, 0, NULL, ZVAL_PTR_DTOR, 0);

    zend_object_std_init(&request->std, ce);
    request->std.handlers = &lxx_request_handlers;

    return &request->std;
}

static lxx_request_t *lxx_request_fetch(zend_object *object) {
    return (lxx_request_t *) ((char *) object - lxx_request_handlers.offset);
}

static void lxx_request_free(zend_object *object) {
    lxx_request_t *request = lxx_request_fetch(object);

    if (request->base_uri) {
        zend_string_release(request->base_uri);
    }

    if (request->request_uri) {
        zend_string_release(request->request_uri);
    }

    if (request->params) {
		if (GC_DELREF(request->params) == 0) {
			GC_REMOVE_FROM_BUFFER(request->params);
			zend_array_destroy(request->params);
		}
	}

    zend_object_std_dtor(object);
}

void lxx_request_instance(zval *this_ptr) {
    object_init_ex(this_ptr, lxx_request_ce);
}

zval *lxx_request_get_server(char *key, size_t len) {
    zval *ret = NULL;
    if (PG(auto_globals_jit)) {
        zend_is_auto_global_str(ZEND_STRL("_SERVER"));
    }
    zval *_server = &PG(http_globals)[TRACK_VARS_SERVER];
    ret = zend_hash_str_find(Z_ARRVAL_P(_server),key, len);
    return ret;
}

zend_string *lxx_request_get_method(zend_object *object) {
    lxx_request_t *request = lxx_request_fetch(object);

    if (request->method) {
        return zend_string_copy(request->method);
    }

    if (SG(request_info).request_method) {
        request->method = zend_string_init(SG(request_info).request_method, strlen(SG(request_info).request_method), 0);
    } else if (strncasecmp(sapi_module.name, "cli", 3) == 0) {
        request->method = zend_string_init("CLI", sizeof("CLI") - 1, 0);
    } else {
        request->method = zend_string_init("UNKNOW", sizeof("UNKNOW") - 1, 0);
    }

    return zend_string_copy(request->method);;
}

zend_string *lxx_request_get_base_uri(zend_object *object) {
	zval *uri;

    lxx_request_t *request = lxx_request_fetch(object);
    if (request->base_uri) {
        return zend_string_copy(request->base_uri);
    }

	uri = lxx_request_get_server(ZEND_STRL("REQUEST_URI"));

    if (!uri) {
        return NULL;
    }

	char *pos;
    
	if ((pos = strstr(Z_STRVAL_P(uri), "?"))) {
		request->base_uri = zend_string_init(Z_STRVAL_P(uri), pos - Z_STRVAL_P(uri), 0);
	} else {
		request->base_uri = zend_string_init(Z_STRVAL_P(uri), Z_STRLEN_P(uri), 0);
	}

	return zend_string_copy(request->base_uri);
}

zend_string *lxx_request_get_request_uri(zend_object *object) {
    zval *query;

    lxx_request_t *request = lxx_request_fetch(object);

    if (request->request_uri) {
        return zend_string_copy(request->request_uri);
    }
    query = lxx_request_get_server(ZEND_STRL("REQUEST_URI"));

    if (query) {
        request->request_uri = zend_string_init(Z_STRVAL_P(query), Z_STRLEN_P(query), 0);
    } else {
        return NULL;
    }

    return zend_string_copy(request->request_uri);
}

void lxx_request_set_params(zend_object *object, zval *subparts) {
    zval *val;
    zend_string *key;
    HashTable *ht;

    lxx_request_t *request = lxx_request_fetch(object);
    ht = Z_ARRVAL_P(subparts);

    ZEND_HASH_FOREACH_STR_KEY_VAL(ht, key, val) {
        if (!key) {
            continue;
        }
        Z_ADDREF_P(val);
        zend_hash_update(request->params, key, val);
    } ZEND_HASH_FOREACH_END();

    // zval *p;
    // ZVAL_ARR(p, request->params);
    // php_var_dump(p,2);
}

ZEND_METHOD(lxx_request, getBaseUri) {

}

ZEND_METHOD(lxx_request, setBaseUri) {

}

ZEND_METHOD(lxx_request, getQueryUri) {

}

ZEND_METHOD(lxx_request, setQueryUri) {

}

ZEND_METHOD(lxx_request, getMethod) {

}

ZEND_METHOD(lxx_request, setMethod) {

}

ZEND_METHOD(lxx_request, getParams) {

}


static zend_function_entry lxx_request_methods[] = {

    ZEND_FE_END
};

LXX_MINIT_FUNCTION(request) {
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Lxx\\Request", lxx_request_methods);
    lxx_request_ce = zend_register_internal_class(&ce);
    lxx_request_ce->ce_flags |= ZEND_ACC_FINAL;
    lxx_request_ce->create_object = lxx_request_new;

    memcpy(&lxx_request_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    lxx_request_handlers.offset = XtOffsetOf(lxx_request_t, std);
    lxx_request_handlers.free_obj = lxx_request_free;

    return SUCCESS;
}