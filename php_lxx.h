/* lxx extension for PHP */

#ifndef PHP_LXX_H
# define PHP_LXX_H

extern zend_module_entry lxx_module_entry;
# define phpext_lxx_ptr &lxx_module_entry

# define PHP_LXX_VERSION "0.1.0"

#define LXX_STARTUP(module)                 ZEND_MODULE_STARTUP_N(lxx_##module)(INIT_FUNC_ARGS_PASSTHRU)
#define LXX_MINIT_FUNCTION(module)          ZEND_MINIT_FUNCTION(lxx_##module)

#define THIS_P								Z_OBJ_P(getThis())

#if PHP_VERSION_ID < 70400
#define LXX_WRITE_HANDLER       void
#define LXX_WHANDLER_RET(zv)    return
# if PHP_VERSION_ID < 70300
# define GC_ADDREF(gc)           (++GC_REFCOUNT(gc))
# define GC_DELREF(gc)           (--GC_REFCOUNT(gc))
# endif
#else
#define LXX_WRITE_HANDLER       zval *
#define LXX_WHANDLER_RET(zv)    return zv
#endif

#if PHP_VERSION_ID < 80000
#define lxx_object zval
#else
#define lxx_object zend_object
#endif


ZEND_BEGIN_MODULE_GLOBALS(lxx)
	zend_string *app_dir;
	zval		app;
ZEND_END_MODULE_GLOBALS(lxx)

extern ZEND_DECLARE_MODULE_GLOBALS(lxx);

#ifdef ZTS
#define LXX_G(v) TSRMG(lxx_globals_id, zend_lxx_globals *, v)
#else
#define LXX_G(v) (lxx_globals.v)
#endif

# if defined(ZTS) && defined(COMPILE_DL_LXX)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_LXX_H */

