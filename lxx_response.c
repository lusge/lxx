#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "main/SAPI.h"
#include "ext/json/php_json.h"
#include "Zend/zend_string.h"
#include "ext/standard/php_string.h"
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


/**
 * copy ext/standard/array.c line 1650 . 
 * static zend_always_inline int php_valid_var_name(const char *var_name, size_t var_name_len) 
 */
static zend_always_inline int lxx_response_valid_var_name(const char *var_name, size_t var_name_len) /* {{{ */
{
	size_t i;
	uint32_t ch;

	if (UNEXPECTED(!var_name_len)) {
		return 0;
	}

	/* These are allowed as first char: [a-zA-Z_\x7f-\xff] */
	ch = (uint32_t)((unsigned char *)var_name)[0];
	if (var_name[0] != '_' &&
		(ch < 65  /* A    */ || /* Z    */ ch > 90)  &&
		(ch < 97  /* a    */ || /* z    */ ch > 122) &&
		(ch < 127 /* 0x7f */ || /* 0xff */ ch > 255)
	) {
		return 0;
	}

	/* And these as the rest: [a-zA-Z0-9_\x7f-\xff] */
	if (var_name_len > 1) {
		i = 1;
		do {
			ch = (uint32_t)((unsigned char *)var_name)[i];
			if (var_name[i] != '_' &&
				(ch < 48  /* 0    */ || /* 9    */ ch > 57)  &&
				(ch < 65  /* A    */ || /* Z    */ ch > 90)  &&
				(ch < 97  /* a    */ || /* z    */ ch > 122) &&
				(ch < 127 /* 0x7f */ || /* 0xff */ ch > 255)
			) {
				return 0;
			}
		} while (++i < var_name_len);
	}
	return 1;
}

static zend_long lxx_response_extract_ref_overwrite(zend_array *arr, zend_array *symbol_table) /* {{{ */
{
	zend_long count = 0;
	zend_string *var_name;
	zval *entry, *orig_var;

	ZEND_HASH_FOREACH_STR_KEY_VAL_IND(arr, var_name, entry) {
		if (!var_name) {
			continue;
		}
		if (!lxx_response_valid_var_name(ZSTR_VAL(var_name), ZSTR_LEN(var_name))) {
			continue;
		}
		if (zend_string_equals_literal(var_name, "this")) {
			zend_throw_error(NULL, "Cannot re-assign $this");
			return -1;
		}
		orig_var = zend_hash_find_ex(symbol_table, var_name, 1);
		if (orig_var) {
			if (Z_TYPE_P(orig_var) == IS_INDIRECT) {
				orig_var = Z_INDIRECT_P(orig_var);
			}
			if (zend_string_equals_literal(var_name, "GLOBALS")) {
				continue;
			}
			if (Z_ISREF_P(entry)) {
				Z_ADDREF_P(entry);
			} else {
				ZVAL_MAKE_REF_EX(entry, 2);
			}
			zval_ptr_dtor(orig_var);
			ZVAL_REF(orig_var, Z_REF_P(entry));
		} else {
			if (Z_ISREF_P(entry)) {
				Z_ADDREF_P(entry);
			} else {
				ZVAL_MAKE_REF_EX(entry, 2);
			}
			zend_hash_add_new(symbol_table, var_name, entry);
		}
		count++;
	} ZEND_HASH_FOREACH_END();

	return count;
}

static int lxx_response_view_ob(zend_op_array *op_array, zval *retval) {
	zval result;
	ZVAL_UNDEF(&result);
	if (php_output_start_user(NULL, 0, PHP_OUTPUT_HANDLER_STDFLAGS) == FAILURE) {
		php_error_docref("ref.outcontrol", E_WARNING, "failed to create buffer");
		return 0;
	}
	
	zend_execute(op_array, &result);
	zval_ptr_dtor(&result);

	if (EG(exception) != NULL) {
		php_output_discard();
	}

	if (php_output_get_contents(retval) == FAILURE) {
		php_output_end();
		php_error_docref(NULL, E_WARNING, "Unable to fetch ob content");
		return 0;
	}

	if (php_output_discard() != SUCCESS ) {
		return 0;
	}
	return 1;
}

void lxx_response_view(zend_object *object, zend_string *filename, zval *arr) {
    zend_string *path;
    zend_array *symbol_table;
	int status = 0;
	zend_file_handle file_handle;
	zend_op_array 	*op_array;
	char realpath[MAXPATHLEN];

    zval retval;

    symbol_table = zend_rebuild_symbol_table();

    if (arr) {
        if (zend_hash_num_elements(Z_ARRVAL_P(arr))) {
            (void)lxx_response_extract_ref_overwrite(Z_ARRVAL_P(arr), symbol_table);
        }
    }
    
    zend_string *file_trim = php_trim(filename, (char *)"/", sizeof("/")-1, 3);

    path = strpprintf(0, "%s/views/%s.php", ZSTR_VAL(LXX_G(app_dir)), ZSTR_VAL(file_trim));
	zend_string_release(file_trim);

	if (!VCWD_REALPATH(ZSTR_VAL(path), realpath)) {
		zend_string_release(path);
		php_error(E_ERROR, "Failed opening template %s: %s", ZSTR_VAL(path), strerror(errno));
		return;
	}

#if PHP_VERSION_ID < 70400
	file_handle.filename = ZSTR_VAL(path);
	file_handle.free_filename = 0;
	file_handle.type = ZEND_HANDLE_FILENAME;
	file_handle.opened_path = NULL;
	file_handle.handle.fp = NULL;
#else
	/* setup file-handle */
	zend_stream_init_filename(&file_handle, tpl);
#endif
	op_array = zend_compile_file(&file_handle, ZEND_INCLUDE);

	if (op_array) {
		if (file_handle.handle.stream.handle) {
			if (!file_handle.opened_path) {
				file_handle.opened_path = zend_string_copy(path);
			}
			zend_hash_add_empty_element(&EG(included_files), file_handle.opened_path);
		}

		status = lxx_response_view_ob(op_array, &retval);

        if ( status == 0) {
            // zend_printf(" sldjflksdjflksdjfls ");
        } else {
            lxx_response_send(object, Z_STRVAL(retval), Z_STRLEN(retval));
        }

		destroy_op_array(op_array);
		efree(op_array);
	} 
	zend_destroy_file_handle(&file_handle);
	zend_string_release(path);
    zval_ptr_dtor(&retval);
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