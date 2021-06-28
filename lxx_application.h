#ifndef LXX_APPLICATION_H
#define LXX_APPLICATION_H

#define LXX_APPLICATION_APP         "app"
#define LXX_APPLICATION_ROUTE_DIR   "/routers/router.php"

#define LXXAPPOBJ()           (lxx_application_fetch(Z_OBJ(LXX_G(app))))

typedef struct {
    zval        router;
    zval        response;
    zend_object std;
} lxx_application_t;

lxx_application_t *lxx_application_fetch(zend_object *object);

LXX_MINIT_FUNCTION(application);

#endif