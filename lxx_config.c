#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"

#include "php_lxx.h"
#include "lxx_config.h"
#include "lxx_loader.h"

zend_class_entry *lxx_config_ce;
static zend_object_handlers lxx_config_handlers;


static zend_object *lxx_config_new(zend_class_entry *ce) {
    lxx_config_t *config = emalloc(sizeof(lxx_config_t) + zend_object_properties_size(ce));
    memset(config, 0, XtOffsetOf(lxx_config_t, std));

    ALLOC_HASHTABLE(config->config);
    zend_hash_init(config->config, 0, NULL, ZVAL_PTR_DTOR, 0);

    zend_object_std_init(&config->std, ce);
    config->std.handlers = &lxx_config_handlers;

    return &config->std;
}

static lxx_config_t *lxx_config_fetch(zend_object *object) {
    return (lxx_config_t *) ((char *) object - lxx_config_handlers.offset);
}

static void lxx_config_free(zend_object *object) {
    lxx_config_t *config = lxx_config_fetch(object);

    if (config->config) {
		if (GC_DELREF(config->config) == 0) {
			GC_REMOVE_FROM_BUFFER(config->config);
			zend_array_destroy(config->config);
		}
	}

    zend_object_std_dtor(object);
}

void lxx_config_instance(zval *this_ptr) {
    zval retval;
    object_init_ex(this_ptr, lxx_config_ce);

    lxx_config_t *config = lxx_config_fetch(Z_OBJ_P(this_ptr));

    zend_string *config_dir = strpprintf(0, "%s%s", ZSTR_VAL(LXX_G(app_dir)), LXX_CONFIG_DIR);
    lxx_loader_include(config_dir, &retval);
    zend_string_release(config_dir);

    zend_hash_copy(config->config, Z_ARRVAL_P(&retval), (copy_ctor_func_t)zval_add_ref);
    
    zval_ptr_dtor(&retval);
}

zval *lxx_config_get_key_value(zend_object *object, const char *key) {
    zval *retval;
    lxx_config_t *conf = lxx_config_fetch(object);
    retval = zend_hash_str_find(conf->config, key, strlen(key));
    return retval;
}

zend_function_entry lxx_config_methods[] = {

    ZEND_FE_END
};

LXX_MINIT_FUNCTION(config) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Lxx\\Config", lxx_config_methods);
    lxx_config_ce = zend_register_internal_class(&ce);
    lxx_config_ce->ce_flags |= ZEND_ACC_FINAL;
    lxx_config_ce->create_object = lxx_config_new;

    memcpy(&lxx_config_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    lxx_config_handlers.offset = XtOffsetOf(lxx_config_t, std);
    lxx_config_handlers.free_obj = lxx_config_free;

    return SUCCESS;
}