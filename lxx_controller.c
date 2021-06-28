#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"

#include "php_lxx.h"
#include "lxx_controller.h"

zend_class_entry *lxx_controller_ce;
static zend_object_handlers lxx_controller_handlers;


static zend_object *lxx_controller_new(zend_class_entry *ce) {
    lxx_controller_t *controller = emalloc(sizeof(lxx_controller_t) + zend_object_properties_size(ce));
    memset(controller, 0, XtOffsetOf(lxx_controller_t, std));


    zend_object_std_init(&controller->std, ce);
    controller->std.handlers = &lxx_controller_handlers;

    return &controller->std;
}

static lxx_controller_t *lxx_controller_fetch(zend_object *object) {
    return (lxx_controller_t *) ((char *) object - lxx_controller_handlers.offset);
}

static void lxx_controller_free(zend_object *object) {
    lxx_controller_t *controller = lxx_controller_fetch(object);

    zval_ptr_dtor(&controller->router);
    zval_ptr_dtor(&controller->request);
    zval_ptr_dtor(&controller->response);

    zend_object_std_dtor(object);
}

void lxx_controller_set_router(zend_object *object, zval *router) {
    lxx_controller_t *controller = lxx_controller_fetch(object);
    ZVAL_COPY(&controller->router, router);
}

void lxx_controller_set_request(zend_object *object, zval *request) {
    lxx_controller_t *controller = lxx_controller_fetch(object);
    ZVAL_COPY(&controller->request, request);
}

void lxx_controller_set_response(zend_object *object, zval *response) {
    lxx_controller_t *controller = lxx_controller_fetch(object);
    ZVAL_COPY(&controller->response, response);
}

ZEND_BEGIN_ARG_INFO_EX(lxx_controller_Prepare_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(lxx_controller, Prepare) {
    // lxx_controller_t *controller = lxx_controller_fetch(Z_OBJ_P(getThis()));
    // php_var_dump(&controller->request,2);
    // php_var_dump(&controller->router,2);
}

ZEND_BEGIN_ARG_INFO_EX(lxx_controller_after_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(lxx_controller, After) {
    
}

zend_function_entry lxx_controller_methods[] = {
    ZEND_ME(lxx_controller, Prepare, lxx_controller_Prepare_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_controller, After, lxx_controller_after_arginfo, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

LXX_MINIT_FUNCTION(controller) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Lxx\\Controller", lxx_controller_methods);
    lxx_controller_ce = zend_register_internal_class(&ce);
    lxx_controller_ce->create_object = lxx_controller_new;

    memcpy(&lxx_controller_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    lxx_controller_handlers.offset = XtOffsetOf(lxx_controller_t, std);
    lxx_controller_handlers.free_obj = lxx_controller_free;

    return SUCCESS;
}