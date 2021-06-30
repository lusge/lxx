#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "main/SAPI.h"
#include "ext/json/php_json.h"
#include "Zend/zend_string.h"
#include "Zend/zend_smart_str.h"

#include "php_lxx.h"
#include "lxx_response.h"

zend_class_entry *lxx_response_ce;
static zend_object_handlers lxx_response_handlers;


static zend_object *lxx_response_new(zend_class_entry *ce) {
    lxx_response_t *response = emalloc(sizeof(lxx_response_t) + zend_object_properties_size(ce));
    memset(response, 0, XtOffsetOf(lxx_response_t, std));

    ALLOC_HASHTABLE(response->header);
    zend_hash_init(response->header, 0, NULL, ZVAL_PTR_DTOR, 0);

    response->code = 200;

    zend_object_std_init(&response->std, ce);
    response->std.handlers = &lxx_response_handlers;

    return &response->std;
}

static lxx_response_t *lxx_response_fetch(zend_object *object) {
    return (lxx_response_t *) ((char *) object - lxx_response_handlers.offset);
}

static void lxx_response_free(zend_object *object) {
    lxx_response_t *response = lxx_response_fetch(object);

    if (response->header) {
		if (GC_DELREF(response->header) == 0) {
			GC_REMOVE_FROM_BUFFER(response->header);
			zend_array_destroy(response->header);
		}
	}

    zend_object_std_dtor(object);
}

void lxx_response_instance(zval *this_ptr) {
    object_init_ex(this_ptr, lxx_response_ce);
}

static void lxx_response_send_header(lxx_response_t *response) {
    zend_string *key;
    zval        *val;
    zend_ulong  nkey;
    sapi_header_line ctr = {0};

    if (response->code) {
        SG(sapi_headers).http_response_code = response->code;
    }

    zval rc;
    ZVAL_ARR(&rc, response->header);
    
    ZEND_HASH_FOREACH_KEY_VAL(response->header, nkey, key, val) {
        if (key) {
            ctr.line_len = spprintf((char**)&(ctr.line), 0, "%s: %s", ZSTR_VAL(key), Z_STRVAL_P(val));
        } else {
            ctr.line_len = spprintf((char**)&(ctr.line), 0, ""ZEND_ULONG_FMT": %s", nkey, Z_STRVAL_P(val));
        }
        ctr.response_code = 0;
        if (sapi_header_op(SAPI_HEADER_REPLACE, &ctr) != SUCCESS) {
            efree((char*)ctr.line);
            return;
        }
    } ZEND_HASH_FOREACH_END();
    efree((char*)ctr.line);
}

void lxx_response_set_hader(zend_object *object, zend_string *key, zend_string *value) {
    zval pData;
    lxx_response_t *response = lxx_response_fetch(object);
    
    ZVAL_STR_COPY(&pData, value);
    zend_hash_add(response->header, key, &pData);
}

void lxx_response_json(zend_object *object, zval *arr) {
    lxx_response_t *response = lxx_response_fetch(object);
    zend_string *key;
    zend_string *value;
    key = zend_string_init("Content-type", sizeof("Content-type") - 1, 0);
    value = zend_string_init("application/json", sizeof("application/json") - 1, 0);

    lxx_response_set_hader(object, key, value);
    lxx_response_send_header(response);

    JSON_G(error_code) = PHP_JSON_ERROR_NONE;
    JSON_G(encode_max_depth) = 512;

    smart_str buf = { 0 };

    php_json_encode(&buf, arr, PHP_JSON_UNESCAPED_UNICODE);

    if (JSON_G(error_code) != PHP_JSON_ERROR_NONE && !(PHP_JSON_UNESCAPED_UNICODE & PHP_JSON_PARTIAL_OUTPUT_ON_ERROR))
    {
        smart_str_free(&buf);
        return;
    }
    else
    {
        smart_str_0(&buf);
        php_write(ZSTR_VAL(buf.s), ZSTR_LEN(buf.s));
        smart_str_free(&buf);
    }

    zend_string_release(key);
    zend_string_release(value);
}

void lxx_response_view(zend_object *object, zend_string *file, zval *arr) {

}

void lxx_response_text(zend_object *object, zend_string *text) {
    lxx_response_t *response = lxx_response_fetch(object);
    zend_string *key;
    zend_string *value;
    key = zend_string_init("Content-type", sizeof("Content-type") - 1, 0);
    value = zend_string_init("text/plain", sizeof("text/plain") - 1, 0);

    lxx_response_set_hader(object, key, value);
    lxx_response_send_header(response);
    php_write(ZSTR_VAL(text), ZSTR_LEN(text));

    zend_string_release(key);
    zend_string_release(value);
}

void lxx_response_html(zend_object *object, zend_string *html) {
    lxx_response_t *response = lxx_response_fetch(object);
    zend_string *key;
    zend_string *value;
    key = zend_string_init("Content-type", sizeof("Content-type") - 1, 0);
    value = zend_string_init("text/html; charset=utf-8", sizeof("text/html; charset=utf-8") - 1, 0);

    lxx_response_set_hader(object, key, value);
    lxx_response_send_header(response);
    php_write(ZSTR_VAL(html), ZSTR_LEN(html));

    zend_string_release(key);
    zend_string_release(value);
}

void lxx_response_send(zend_object *object, char *body, size_t len) {
    lxx_response_t *response = lxx_response_fetch(object);
    lxx_response_send_header(response);
    php_write(body, len);
}

zend_function_entry lxx_response_methods[] = {

    ZEND_FE_END
};

LXX_MINIT_FUNCTION(response) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Lxx\\Response", lxx_response_methods);
    lxx_response_ce = zend_register_internal_class(&ce);
    lxx_response_ce->ce_flags |= ZEND_ACC_FINAL;
    lxx_response_ce->create_object = lxx_response_new;

    memcpy(&lxx_response_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    lxx_response_handlers.offset = XtOffsetOf(lxx_response_t, std);
    lxx_response_handlers.free_obj = lxx_response_free;

    return SUCCESS;
}