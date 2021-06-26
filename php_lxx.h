/* lxx extension for PHP */

#ifndef PHP_LXX_H
# define PHP_LXX_H

extern zend_module_entry lxx_module_entry;
# define phpext_lxx_ptr &lxx_module_entry

# define PHP_LXX_VERSION "0.1.0"

#define LXX_STARTUP(module)                 ZEND_MODULE_STARTUP_N(lxx_##module)(INIT_FUNC_ARGS_PASSTHRU)
#define LXX_MINIT_FUNCTION(module)          ZEND_MINIT_FUNCTION(lxx_##module)

# if defined(ZTS) && defined(COMPILE_DL_LXX)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_LXX_H */

