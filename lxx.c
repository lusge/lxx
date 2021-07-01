/* lxx extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_lxx.h"

#include "lxx_router.h"
#include "lxx_request.h"
#include "lxx_application.h"
#include "lxx_config.h"
#include "lxx_controller.h"
#include "lxx_loader.h"
#include "lxx_response.h"
#include "lxx_exception.h"

ZEND_DECLARE_MODULE_GLOBALS(lxx);

/* 初始化module时运行 */
PHP_MINIT_FUNCTION(lxx)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	LXX_STARTUP(application);
	LXX_STARTUP(config);
	LXX_STARTUP(controller);
	LXX_STARTUP(loader);
	LXX_STARTUP(response);
	LXX_STARTUP(request);
	LXX_STARTUP(router);
	LXX_STARTUP(exception);
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
	if (LXX_G(app_dir)) {
		zend_string_release(LXX_G(app_dir));
	}
	
	zval_ptr_dtor(&LXX_G(app));
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

/* {{{ lxx_module_entry
 */
zend_module_entry lxx_module_entry = {
	STANDARD_MODULE_HEADER,
	"lxx",					/* Extension name */
	NULL,			/* zend_function_entry */
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

