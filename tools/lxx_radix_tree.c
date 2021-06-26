#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"

#include "php_lxx.h"
#include "tools/lxx_radix_tree.h"

zend_class_entry *lxx_rax_tree_ce;
static zend_object_handlers lxx_rax_tree_handlers;

static zend_object *lxx_rax_tree_new(zend_class_entry *ce) {
    rax_tree_t *tree = emalloc(sizeof(rax_tree_t) + zend_object_properties_size(ce));
    memset(tree, 0, XtOffsetOf(rax_tree_t, std));

    tree->radix = raxNew();
    
    raxStart(&tree->it, tree->radix);
    zend_object_std_init(&tree->std, ce);
    object_properties_init(&tree->std, ce);
    tree->std.handlers = &lxx_rax_tree_handlers;
    return &tree->std;
}

static rax_tree_t *lxx_rax_tree_fetch(zend_object *object) {
    return (rax_tree_t *) ((char *) object - lxx_rax_tree_handlers.offset);
}

static void lxx_rax_tree_free(zend_object *object) {
    rax_tree_t *tree = lxx_rax_tree_fetch(object);
    raxStop(&tree->it);
    raxFree(tree->radix);
    zend_object_std_dtor(object);
}


void lxx_rax_tree_instance(zval *rax_tree) {
    object_init_ex(rax_tree, lxx_rax_tree_ce);
}

void lxx_rax_tree_show(zend_object *object) {
    rax_tree_t *tree = lxx_rax_tree_fetch(object);
    raxShow(tree->radix);
}

zend_long lxx_rax_tree_find(zend_object *object, zend_string *key) {
    rax_tree_t *tree = lxx_rax_tree_fetch(object);
    void *data = raxFind(tree->radix, (unsigned char *)ZSTR_VAL(key), ZSTR_LEN(key));
    if (data == raxNotFound) {
        
        return -1;
    }
    
    return (zend_long)data;
}

int lxx_rax_tree_insert(zend_object *object, zend_string *key, zend_long val) {
    rax_tree_t *tree = lxx_rax_tree_fetch(object);
    // zend_printf(" new_path %s and len = %d \n", (unsigned char *)ZSTR_VAL(key), ZSTR_LEN(key));
    return raxInsert(tree->radix, (unsigned char *)ZSTR_VAL(key), ZSTR_LEN(key), (void *)val, NULL);;
}

int lxx_rax_tree_seek(zend_object *object, zend_string *op, zend_string *ele) {
    rax_tree_t *tree = lxx_rax_tree_fetch(object);
    
    return raxSeek(&tree->it, ZSTR_VAL(op), (unsigned char *)ZSTR_VAL(ele), ZSTR_LEN(ele));
}

zend_long lxx_rax_tree_pre(zend_object *object) {
    rax_tree_t *tree = lxx_rax_tree_fetch(object);
    int res = raxPrev(&tree->it);
    if (!res) {
        return -1;
    }

    return (zend_long)tree->it.data;
}

zend_long lxx_rax_tree_next(zend_object *object) {
    rax_tree_t *tree = lxx_rax_tree_fetch(object);
    int res = raxNext(&tree->it);
    if (!res) {
        return -1;
    }

    return (zend_long)tree->it.data;
}

zend_long lxx_rax_tree_pre_seach(zend_object *object, const char *op, zend_string *key) {
    rax_tree_t *tree = lxx_rax_tree_fetch(object);
    int res;
    raxSeek(&tree->it, op, (unsigned char *)ZSTR_VAL(key), ZSTR_LEN(key));
    while (1) {
        res = raxPrev(&tree->it);
        if (!res) {
            return -1;
        }

        if (tree->it.key_len > ZSTR_LEN(key) || memcmp(ZSTR_VAL(key), tree->it.key, tree->it.key_len) != 0) {
            continue;
        }
        break;
    }

    return (zend_long)tree->it.data;
}

zend_long lxx_rax_tree_nex_seach(zend_object *object, const char *op, zend_string *key) {
    rax_tree_t *tree = lxx_rax_tree_fetch(object);
    int res;
    raxSeek(&tree->it, op, (unsigned char *)ZSTR_VAL(key), ZSTR_LEN(key));
    while (1) {
        res = raxPrev(&tree->it);
        if (!res) {
            return -1;
        }

        if (tree->it.key_len > ZSTR_LEN(key) || memcmp(ZSTR_VAL(key), tree->it.key, tree->it.key_len) != 0) {
            continue;
        }
        break;
    }

    return (zend_long)tree->it.data;
}


ZEND_METHOD(lxx_rax_tree, __construct) {

}

ZEND_METHOD(lxx_rax_tree, insert) {
    zend_string *key;
    zend_long val;
    int ret;


    if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sl", &key, &val) == FAILURE) {
        return;
    }

    ret = lxx_rax_tree_insert(Z_OBJ_P(getThis()), key, val);
    if (ret == 0) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

ZEND_METHOD(lxx_rax_tree, find) {
    zend_string *key;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &key) == FAILURE) {
        return;
    }

    zend_long data = lxx_rax_tree_find(Z_OBJ_P(getThis()), key);

    if (data == -1) {
        RETURN_FALSE;
    }

    RETURN_LONG(data);
}

ZEND_METHOD(lxx_rax_tree, seek) {
    zend_string *op, *ele;
    int ret;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS", &op, &ele) == FAILURE) {
        return;
    }
    
    ret = lxx_rax_tree_seek(Z_OBJ_P(getThis()), op, ele);
    
    if (ret == 0 || ret == ENOMEM) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

ZEND_METHOD(lxx_rax_tree, pre) {
    zend_long res;
    
    res = lxx_rax_tree_pre(Z_OBJ_P(getThis()));

    if (res == -1) {
        RETURN_FALSE;
    }
    
    RETURN_LONG(res);
}

ZEND_METHOD(lxx_rax_tree, next) {
    zend_long res;
    
    res = lxx_rax_tree_next(Z_OBJ_P(getThis()));

    if (res == -1) {
        RETURN_FALSE;
    }
    
    RETURN_LONG(res);
}

ZEND_BEGIN_ARG_INFO_EX(rax_tree_arg, 0, 0, 2)
    ZEND_ARG_INFO(0, op)
    ZEND_ARG_INFO(0, buf)
ZEND_END_ARG_INFO()

zend_function_entry lxx_rax_tree_methods[] = {
    ZEND_ME(lxx_rax_tree, __construct, NULL, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_rax_tree, insert, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_rax_tree, seek, rax_tree_arg, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_rax_tree, find, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_rax_tree, pre, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(lxx_rax_tree, next, NULL, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};


LXX_MINIT_FUNCTION(raxTree) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Lxx\\RadixTree", lxx_rax_tree_methods);
    lxx_rax_tree_ce = zend_register_internal_class(&ce);
    lxx_rax_tree_ce->ce_flags |= ZEND_ACC_FINAL;
    lxx_rax_tree_ce->create_object = lxx_rax_tree_new;

    memcpy(&lxx_rax_tree_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    lxx_rax_tree_handlers.offset = XtOffsetOf(rax_tree_t, std);
    lxx_rax_tree_handlers.free_obj = lxx_rax_tree_free;
    lxx_rax_tree_handlers.clone_obj = NULL;

    return SUCCESS;
}