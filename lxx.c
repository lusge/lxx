/* lxx extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_lxx.h"

#include "lxx_router.h"
#include "tools/lxx_radix_tree.h"
#include "lxx_request.h"


/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

/* {{{ void lxx_test1()
 */
PHP_FUNCTION(lxx_test1)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_printf("The extension %s is loaded and working!\r\n", "lxx");
}
/* }}} */

/* {{{ string lxx_test2( [ string $var ] )
 */
PHP_FUNCTION(lxx_test2)
{
	char *var = "World";
	size_t var_len = sizeof("World") - 1;
	zend_string *retval;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(var, var_len)
	ZEND_PARSE_PARAMETERS_END();

	retval = strpprintf(0, "Hello %s", var);

	RETURN_STR(retval);
}
/* }}}*/

/* 初始化module时运行 */
PHP_MINIT_FUNCTION(lxx)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/

	LXX_STARTUP(request);
	LXX_STARTUP(raxTree);
	LXX_STARTUP(router);
	return SUCCESS;
}

/* 当module被卸载时运行 */
PHP_MSHUTDOWN_FUNCTION(lxx)
{
    /* uncomment this line if you have INI entries
    UNREGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(lxx)
{
#if defined(ZTS) && defined(COMPILE_DL_LXX)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/*当一个REQUEST请求结束时运行*/
PHP_RSHUTDOWN_FUNCTION(lxx)
{
    return SUCCESS;
}

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(lxx)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "lxx support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ arginfo
 */
ZEND_BEGIN_ARG_INFO(arginfo_lxx_test1, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_lxx_test2, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ lxx_functions[]
 */
static const zend_function_entry lxx_functions[] = {
	PHP_FE(lxx_test1,		arginfo_lxx_test1)
	PHP_FE(lxx_test2,		arginfo_lxx_test2)
	PHP_FE_END
};
/* }}} */

/* {{{ lxx_module_entry
 */
zend_module_entry lxx_module_entry = {
	STANDARD_MODULE_HEADER,
	"lxx",					/* Extension name */
	lxx_functions,			/* zend_function_entry */
	PHP_MINIT(lxx),			/* PHP_MINIT - Module initialization */
	PHP_MSHUTDOWN(lxx),		/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(lxx),			/* PHP_RINIT - Request initialization */
	PHP_RSHUTDOWN(lxx),		/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(lxx),			/* PHP_MINFO - Module info */
	PHP_LXX_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_LXX
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(lxx)
#endif

