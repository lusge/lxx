#ifndef LXX_APPLICATION_H
#define LXX_APPLICATION_H

#define LXX_APPLICATION_APP     "app"


typedef struct {
    zval        router;
    zval        response;
    zend_object std;
} lxx_application_t;



LXX_MINIT_FUNCTION(application);

#endif