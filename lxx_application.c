#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_string.h"
#include "Zend/zend_interfaces.h"

#include "php_lxx.h"
#include "lxx_application.h"
#include "lxx_router.h"
#include "lxx_loader.h"

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

    if (Z_TYPE(app->router) != IS_UNDEF) {
        OBJ_RELEASE(Z_OBJ(app->router));
    }

    zend_object_std_dtor(object);
}

static void lxx_application_function_handle(zval *this) {
    zval *func;

    lxx_application_t *app = lxx_application_fetch(Z_OBJ_P(this));
    func = lxx_router_match_router(Z_OBJ(app->router));

    if (func) {
        if (Z_TYPE_P(func) == IS_OBJECT) {
            zval retval;
            call_user_function(CG(function_table), NULL, func, &retval, 1, this);
            if (Z_TYPE(retval) == IS_STRING) {
                zend_printf(" %s ", Z_STRVAL(retval));
            }
            zval_ptr_dtor(&retval);

        } else if (Z_TYPE_P(func) == IS_STRING){
            zend_string *controller;
            zend_string *namespace_controller;
            zend_string *action;
            zend_string *lc_action;
            char *pos;
            
            pos = strchr(Z_STRVAL_P(func), '@');
            controller = zend_string_init(Z_STRVAL_P(func), pos - Z_STRVAL_P(func), 0);
            lxx_router_set_controller(Z_OBJ(app->router), controller);

            namespace_controller = strpprintf(0, "controllers\\%s", ZSTR_VAL(controller));
            
            action = zend_string_init(pos+1, Z_STRLEN_P(func) - (pos - Z_STRVAL_P(func)) - 1, 0);
            lxx_router_set_action(Z_OBJ(app->router), action);
            lc_action = zend_string_tolower(action);

            zend_class_entry *ce = zend_lookup_class(namespace_controller);

            if (ce) {
                zval class_object;
                zval ret;
                object_init_ex(&class_object, ce);
                zend_call_method_with_0_params(&class_object, ce, NULL, "before", &ret);

                if (Z_TYPE(ret) != IS_TRUE) {
                    zval_ptr_dtor(&ret);
                    zval_ptr_dtor(&class_object);
                    // zend_printf("true <br>");
                    return;
                }
                // ZVAL_NULL(&ret);
                zend_call_method(&class_object, ce, NULL, ZSTR_VAL(lc_action), ZSTR_LEN(lc_action), &ret, 0, NULL, NULL);
                zval_ptr_dtor(&class_object);
            } else {
                zend_error_noreturn(E_ERROR, "Couldn't find controller and action");
            }

            zend_printf("controller  %s, namespace controller = %s, action = %s, len = %ld", ZSTR_VAL(controller),ZSTR_VAL(namespace_controller), ZSTR_VAL(action), Z_STRLEN_P(func));
            zend_string_release(controller);
            zend_string_release(namespace_controller);
            zend_string_release(action);
            zend_string_release(lc_action);
        } else {
            php_error(E_ERROR, "It can only be string or function in %s on %d ", __FILE__, __LINE__);
        }
    } else {
        php_error(E_ERROR, "Function not found in %s on %d ", __FILE__, __LINE__);
    } 
}

ZEND_BEGIN_ARG_INFO_EX(lxx_application_ctor_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, dir)
ZEND_END_ARG_INFO()

ZEND_METHOD(lxx_application, __construct) {
    zend_string *dir;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &dir) == FAILURE) {
        return;
    }

    LXX_G(app_dir) = php_trim(dir, (char *)"/", sizeof("/")-1, 2);

    lxx_application_t *app = lxx_application_fetch(Z_OBJ_P(getThis()));
    lxx_router_instance(&app->router);

    lxx_loader_instance();
    zend_update_static_property(lxx_application_ce, ZEND_STRL(LXX_APPLICATION_APP), getThis());
}

ZEND_BEGIN_ARG_INFO_EX(lxx_application_app_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(lxx_application, app) {
    zval *app = zend_read_static_property(lxx_application_ce, ZEND_STRL(LXX_APPLICATION_APP), 1);
    RETURN_ZVAL(app, 1, 0);
}

ZEND_BEGIN_ARG_INFO_EX(lxx_application_run_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(lxx_application, run) {
    lxx_application_function_handle(getThis()); 
}

ZEND_BEGIN_ARG_INFO_EX(lxx_application_router_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(lxx_application, router) {
    lxx_application_t *app = lxx_application_fetch(Z_OBJ_P(getThis()));
    RETURN_ZVAL(&app->router, 1, 0);
}

zend_function_entry lxx_application_methods[] = {
    ZEND_ME(lxx_application, __construct, lxx_application_ctor_arginfo, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_application, app, lxx_application_app_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    ZEND_ME(lxx_application, run, lxx_application_run_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_application, router, lxx_application_router_arginfo, ZEND_ACC_PUBLIC)
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

    zend_declare_property_null(lxx_application_ce, ZEND_STRL(LXX_APPLICATION_APP), ZEND_ACC_STATIC | ZEND_ACC_PROTECTED);
    return SUCCESS;
}