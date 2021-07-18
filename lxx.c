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

PHP_INI_BEGIN()
	// STD_PHP_INI_BOOLEAN("lxx.use_swoole",     "Off", PHP_INI_ALL, OnUpdateBool, use_swoole, zend_lxx_globals, lxx_globals)
PHP_INI_END();

PHP_GINIT_FUNCTION(lxx)
{
	memset(lxx_globals, 0, sizeof(*lxx_globals));
}

/* 初始化module时运行 */
PHP_MINIT_FUNCTION(lxx)
{
	REGISTER_INI_ENTRIES();


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
    // UNREGISTER_INI_ENTRIES();
    
    return SUCCESS;
}

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(lxx)
{
#if defined(ZTS) && defined(COMPILE_DL_LXX)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	LXX_G(app_dir) = NULL;
	ZVAL_NULL(&LXX_G(app));
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
	php_info_print_table_row(2, "Author", "卢喜羲 <www.a22c.com> ");
	php_info_print_table_row(2, "Version", PHP_LXX_VERSION);
	
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
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

