#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"

#include "php_lxx.h"
#include "lxx_application.h"

zend_class_entry *lxx_application_ce;
static zend_object_handlers lxx_application_handlers;


static zend_object *lxx_application_new(zend_class_entry *ce) {
    lxx_application_t *app = emalloc(sizeof(lxx_application_t) + zend_object_properties_size(ce));
    memset(app, 0, XtOffsetOf(lxx_application_t, std));


    zend_object_std_init(&app->std, ce);
    app->std.handlers = &lxx_application_handlers;

    return &app->std;
}

static lxx_application_t *lxx_application_fetch(zend_object *object) {
    return (lxx_application_t *) ((char *) object - lxx_application_handlers.offset);
}

static void lxx_application_free(zend_object *object) {
    lxx_application_t *app = lxx_application_fetch(object);

    zend_object_std_dtor(object);
}

zend_function_entry lxx_application_methods[] = {

    ZEND_FE_END
};

LXX_MINIT_FUNCTION(application) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Lxx\\Application", lxx_application_methods);
    lxx_application_ce = zend_register_internal_class(&ce);
    lxx_application_ce->ce_flags |= ZEND_ACC_FINAL;
    lxx_application_ce->create_object = lxx_application_new;

    memcpy(&lxx_application_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    lxx_application_handlers.offset = XtOffsetOf(lxx_application_t, std);
    lxx_application_handlers.free_obj = lxx_application_free;

    return SUCCESS;
}