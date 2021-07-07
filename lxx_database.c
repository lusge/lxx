#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_string.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_smart_str.h" 
#include "ext/pdo/php_pdo_driver.h"

#include "php_lxx.h"
#include "lxx_database.h"
#include "lxx_config.h"
#include "lxx_application.h"
#include "lxx_exception.h"

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

    if (database->table) {
        zend_string_release(database->table);
    }

    zval_ptr_dtor(&database->pdo);

    smart_str_free(&database->where);
    zend_object_std_dtor(object);
}

static void lxx_database_static_instance(zval *this_ptr) {
    object_init_ex(this_ptr, lxx_database_ce);
    zend_call_method_with_0_params(this_ptr, lxx_database_ce, NULL, "__construct", NULL);
}

static void lxx_database_init(zend_object *object) {
    zval *db_config;
    zend_string *key;
    zval *value;
    zend_string *dbname, *host, *driver, *user, *password, *charset, *dns;
    zend_ulong port;

    zval pdo;

    lxx_database_t *database = lxx_database_fetch(object);
    lxx_application_t *app = LXXAPPOBJ();
    
    
    db_config = lxx_config_get_key_value(Z_OBJ(app->config), "db");
    if (!db_config) {
        lxx_throw_exception(E_WARNING, "There is no configuration database .");
        return;
    }

    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(db_config), key, value) {
        if (zend_string_equals_literal(key, "driver")) {
            driver = Z_STR_P(value);
        }

        if (zend_string_equals_literal(key, "host")) {
            host = Z_STR_P(value);
        }

        if (zend_string_equals_literal(key, "dbname")) {
            dbname = Z_STR_P(value);
        }

        if (zend_string_equals_literal(key, "user")) {
            user = Z_STR_P(value);
        }

        if (zend_string_equals_literal(key, "password")) {
            password = Z_STR_P(value);
        }

        if (zend_string_equals_literal(key, "port")) {
            port = Z_LVAL_P(value);
        }

        if (zend_string_equals_literal(key, "charset")) {
            charset = Z_STR_P(value);
        }
    } ZEND_HASH_FOREACH_END();

    dns = strpprintf(0, "%s:host=%s;port=%llu;dbname=%s;charset=%s", ZSTR_VAL(driver), ZSTR_VAL(host), port, ZSTR_VAL(dbname), ZSTR_VAL(charset));
    // zend_printf(ZSTR_VAL(dns));
    object_init_ex(&database->pdo, php_pdo_get_dbh_ce());

    zval ctr, args[3], retval;
    ZVAL_STRING(&ctr, "__construct");

    ZVAL_STR(&args[0], dns);
    ZVAL_STR(&args[1], user);
    ZVAL_STR(&args[2], password);

    call_user_function(&Z_OBJCE(database->pdo)->function_table, &database->pdo, &ctr, &retval, 3, args);

    zval_ptr_dtor(&ctr);
    zval_ptr_dtor(&args[0]);
    zval_ptr_dtor(&args[1]);
    zval_ptr_dtor(&args[2]);
    zval_ptr_dtor(&retval);
}

ZEND_METHOD(lxx_database, __construct) {
    lxx_database_init(Z_OBJ_P(getThis()));
}

ZEND_METHOD(lxx_database, table) {
    zval db;
    zend_string *table;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &table) == FAILURE) {
        return;
    }

    lxx_database_static_instance(&db);
    lxx_database_t *database = lxx_database_fetch(Z_OBJ(db));
    
    if (table) {
        database->table = zend_string_copy(table);

        zend_string_release(table);
    }
    RETURN_ZVAL(&db, 0, 1);
}

ZEND_METHOD(lxx_database, query) {

}

static zend_function_entry lxx_database_methods[] = {
    ZEND_ME(lxx_database, __construct, NULL, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_database, table, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    ZEND_ME(lxx_database, query, NULL, ZEND_ACC_PUBLIC)
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