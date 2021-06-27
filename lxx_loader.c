#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"

#include "php_lxx.h"
#include "lxx_loader.h"

zend_class_entry *lxx_loader_ce;
static zend_object_handlers lxx_loader_handlers;


static zend_object *lxx_loader_new(zend_class_entry *ce) {
    lxx_loader_t *app = emalloc(sizeof(lxx_loader_t) + zend_object_properties_size(ce));
    memset(app, 0, XtOffsetOf(lxx_loader_t, std));


    zend_object_std_init(&app->std, ce);
    app->std.handlers = &lxx_loader_handlers;

    return &app->std;
}

static lxx_loader_t *lxx_loader_fetch(zend_object *object) {
    return (lxx_loader_t *) ((char *) object - lxx_loader_handlers.offset);
}

static void lxx_loader_free(zend_object *object) {
    lxx_loader_t *app = lxx_loader_fetch(object);

    zend_object_std_dtor(object);
}

zend_function_entry lxx_loader_methods[] = {

    ZEND_FE_END
};

LXX_MINIT_FUNCTION(loader) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Lxx\\Loader", lxx_loader_methods);
    lxx_loader_ce = zend_register_internal_class(&ce);
    lxx_loader_ce->ce_flags |= ZEND_ACC_FINAL;
    lxx_loader_ce->create_object = lxx_loader_new;

    memcpy(&lxx_loader_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    lxx_loader_handlers.offset = XtOffsetOf(lxx_loader_t, std);
    lxx_loader_handlers.free_obj = lxx_loader_free;

    return SUCCESS;
}