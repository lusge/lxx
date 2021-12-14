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
#include "lxx_controller.h"
#include "lxx_response.h"
#include "lxx_request.h"
#include "lxx_config.h"
#include "lxx_exception.h"

zend_class_entry *lxx_application_ce;
static zend_object_handlers lxx_application_handlers;


static zend_object *lxx_application_new(zend_class_entry *ce) {
    lxx_application_t *app = emalloc(sizeof(lxx_application_t) + zend_object_properties_size(ce));
    memset(app, 0, XtOffsetOf(lxx_application_t, std));

    zend_object_std_init(&app->std, ce);
    object_properties_init(&app->std, ce);
    app->std.handlers = &lxx_application_handlers;

    return &app->std;
}

lxx_application_t *lxx_application_fetch(zend_object *object) {
    return (lxx_application_t *) ((char *) object - lxx_application_handlers.offset);
}

static void lxx_application_free(zend_object *object) {
    lxx_application_t *app = lxx_application_fetch(object);

    zval_ptr_dtor(&app->router);
    zval_ptr_dtor(&app->response);
    zval_ptr_dtor(&app->request);
    zval_ptr_dtor(&app->config);

    zend_object_std_dtor(object);
}

static void lxx_application_call_function(lxx_application_t *app, zend_string *controller, zend_string *action) {
    zend_string *namespace_controller;
    zend_string *lc_action;

    namespace_controller = strpprintf(0, "controllers\\%s", ZSTR_VAL(controller));
    lc_action = zend_string_tolower(action);

    zend_class_entry *ce = zend_lookup_class(namespace_controller);
    
    if (ce) {
        zval class_object;
        object_init_ex(&class_object, ce);
        
        zend_call_method_with_0_params(&class_object, ce, NULL, "prepare", NULL);
        zend_call_method(&class_object, ce, NULL, ZSTR_VAL(lc_action), ZSTR_LEN(lc_action), NULL, 0, NULL, NULL);
        
        zend_call_method_with_0_params(&class_object, ce, NULL, "after", NULL);
        zval_ptr_dtor(&class_object);
    } else {
        zend_error_noreturn(E_ERROR, "Couldn't find controller and action");
    }

    zend_string_release(namespace_controller);
    zend_string_release(lc_action);
}

static void lxx_application_function_handle(zval *this) {
    zval *func;

    lxx_application_t *app = lxx_application_fetch(Z_OBJ_P(this));
    func = lxx_router_match_router();
// php_var_dump(&app->request, 1);
    if (func) {
        if (Z_TYPE_P(func) == IS_OBJECT) {
            zval retval;
            call_user_function(CG(function_table), NULL, func, &retval, 1, this);
            if (Z_TYPE(retval) == IS_STRING) {
                lxx_response_send(Z_OBJ(app->response), Z_STRVAL(retval), Z_STRLEN(retval));
            }
            zval_ptr_dtor(&retval);

        } else if (Z_TYPE_P(func) == IS_STRING){
            zend_string *controller;
            zend_string *action;
            char *pos;
            
            pos = strchr(Z_STRVAL_P(func), '@');
            controller = zend_string_init(Z_STRVAL_P(func), pos - Z_STRVAL_P(func), 0);
            lxx_router_set_controller(Z_OBJ(app->router), controller);

            action = zend_string_init(pos+1, Z_STRLEN_P(func) - (pos - Z_STRVAL_P(func)) - 1, 0);
            lxx_router_set_action(Z_OBJ(app->router), action);

            lxx_application_call_function(app, controller, action);

            zend_string_release(controller);
            zend_string_release(action);
        } else {
            php_error(E_ERROR, "It can only be string or function in %s on %d ", __FILE__, __LINE__);
        }
    } else {
        php_error(E_ERROR, "Function not found in %s on %d line", __FILE__, __LINE__);
    } 
}

static void lxx_application_load_router_file() {
    size_t len = ZSTR_LEN(LXX_G(app_dir)) + (sizeof(LXX_APPLICATION_ROUTE_DIR) - 1);
    zend_string *router_dir = zend_string_alloc(len , 0);
    memcpy(ZSTR_VAL(router_dir), ZSTR_VAL(LXX_G(app_dir)), ZSTR_LEN(LXX_G(app_dir)));
    memcpy(ZSTR_VAL(router_dir) + ZSTR_LEN(LXX_G(app_dir)), LXX_APPLICATION_ROUTE_DIR, sizeof(LXX_APPLICATION_ROUTE_DIR) - 1);
    ZSTR_VAL(router_dir)[len] = '\0';

    lxx_loader_include(router_dir, NULL);
    zend_string_release(router_dir);
}

static void lxx_application_loader_vendor_autoload_file() {
    size_t len = ZSTR_LEN(LXX_G(app_dir)) + (sizeof(LXX_APPLICATION_VENDOR_DIR) - 1);
    zend_string *vendor_dir = zend_string_alloc(len , 0);
    memcpy(ZSTR_VAL(vendor_dir), ZSTR_VAL(LXX_G(app_dir)), ZSTR_LEN(LXX_G(app_dir)));
    memcpy(ZSTR_VAL(vendor_dir) + ZSTR_LEN(LXX_G(app_dir)), LXX_APPLICATION_VENDOR_DIR, sizeof(LXX_APPLICATION_VENDOR_DIR) - 1);
    ZSTR_VAL(vendor_dir)[len] = '\0';

    char realpath[MAXPATHLEN];
    
	if (!VCWD_REALPATH(ZSTR_VAL(vendor_dir), realpath)) {
        zend_string_release(vendor_dir);
        return;
    }
    lxx_loader_include(vendor_dir, NULL);
    zend_string_release(vendor_dir);
}

static void lxx_application_bootstrap() {
    zend_string *boot = zend_string_init("Bootstrap", sizeof("Bootstrap") - 1, 0);
    zend_class_entry *ce = zend_lookup_class(boot);
    if (ce) {
        zval obj;
        object_init_ex(&obj, ce);
        zend_call_method_with_0_params(&obj, ce, NULL, "init", NULL);
        zval_ptr_dtor(&obj);
    }

    zend_string_release(boot);
}

ZEND_METHOD(lxx_application, __construct) {
    zend_string *dir;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &dir) == FAILURE) {
        return;
    }

    LXX_G(app_dir) = php_trim(dir, (char *)"/", sizeof("/")-1, 2);
    
    lxx_application_t *app = lxx_application_fetch(Z_OBJ_P(getThis()));

    ZVAL_COPY(&LXX_G(app), getThis());

    lxx_router_instance(&app->router);
    lxx_request_instance(&app->request);
    lxx_response_instance(&app->response);
    lxx_loader_instance();
    lxx_config_instance(&app->config);

    lxx_application_load_router_file();
    lxx_application_loader_vendor_autoload_file();
}

ZEND_METHOD(lxx_application, app) {
    RETURN_ZVAL(&LXX_G(app), 1, 0);
}

ZEND_METHOD(lxx_application, run) {

    lxx_application_bootstrap();
    lxx_application_function_handle(getThis()); 
}

// ZEND_METHOD(lxx_application, swCallback) {
//     zval *request, *response;

//     if (zend_parse_parameters(ZEND_NUM_ARGS(), "oo", &request, &response) == FAILURE) {
//         return;
//     }


//     lxx_application_t *app = lxx_application_fetch(Z_OBJ_P(getThis()));

//     // RETURN_ZVAL(&app->router, 1, 0);
// }

ZEND_METHOD(lxx_application, getRouter) {
    lxx_application_t *app = lxx_application_fetch(Z_OBJ_P(getThis()));
    RETURN_ZVAL(&app->router, 1, 0);
}

ZEND_METHOD(lxx_application, getRequest) {
    lxx_application_t *app = lxx_application_fetch(Z_OBJ_P(getThis()));
    RETURN_ZVAL(&app->request, 1, 0);
}

ZEND_METHOD(lxx_application, getConfig) {
    lxx_application_t *app = lxx_application_fetch(Z_OBJ_P(getThis()));
    RETURN_ZVAL(&app->config, 1, 0);
}

ZEND_METHOD(lxx_application, getResponse) {
    lxx_application_t *app = lxx_application_fetch(Z_OBJ_P(getThis()));
    RETURN_ZVAL(&app->response, 1, 0);
}

ZEND_BEGIN_ARG_INFO_EX(lxx_application_ctor_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lxx_application_null_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

zend_function_entry lxx_application_methods[] = {
    ZEND_ME(lxx_application, __construct, lxx_application_ctor_arginfo, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_application, app, lxx_application_null_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    ZEND_ME(lxx_application, run, NULL, ZEND_ACC_PUBLIC)
    // ZEND_ME(lxx_application, swCallback, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_application, getRouter, lxx_application_null_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_application, getRequest, lxx_application_null_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_application, getConfig, lxx_application_null_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_application, getResponse, lxx_application_null_arginfo, ZEND_ACC_PUBLIC)
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