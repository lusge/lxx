#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_string.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_smart_str.h" 

#include "php_lxx.h"
#include "lxx_database.h"



zend_class_entry *lxx_database_ce;
static zend_object_handlers lxx_database_handlers;


static zend_object *lxx_database_new(zend_class_entry *ce) {
    lxx_database_t *database = emalloc(sizeof(lxx_database_t) + zend_object_properties_size(ce));
    memset(database, 0, XtOffsetOf(lxx_database_t, std));

    ALLOC_HASHTABLE(database->execute);
    zend_hash_init(database->execute, 0, NULL, ZVAL_PTR_DTOR, 0);

    zend_object_std_init(&database->std, ce);
    database->std.handlers = &lxx_database_handlers;

    return &database->std;
}

static lxx_database_t *lxx_database_fetch(zend_object *object) {
    return (lxx_database_t *) ((char *) object - lxx_database_handlers.offset);
}

static void lxx_database_free(zend_object *object) {
    lxx_database_t *database = lxx_database_fetch(object);

    if (database->execute) {
		if (GC_DELREF(database->execute) == 0) {
			GC_REMOVE_FROM_BUFFER(database->execute);
			zend_array_destroy(database->execute);
		}
	}

    smart_str_free(&database->where);
    zend_object_std_dtor(object);
}

void lxx_database_instance(zval *this_ptr, zval *db_config) {
    object_init_ex(this_ptr, lxx_database_ce);
}



static zend_function_entry lxx_database_methods[] = {

    ZEND_FE_END
};

LXX_MINIT_FUNCTION(database) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Lxx\\Database", lxx_database_methods);
    lxx_database_ce = zend_register_internal_class(&ce);
    lxx_database_ce->ce_flags |= ZEND_ACC_FINAL;
    lxx_database_ce->create_object = lxx_database_new;

    memcpy(&lxx_database_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    lxx_database_handlers.offset = XtOffsetOf(lxx_database_t, std);
    lxx_database_handlers.free_obj = lxx_database_free;


    return SUCCESS;
}