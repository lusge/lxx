#ifndef LXX_DATABASE_H
#define LXX_DATABASE_H

typedef struct {
    smart_str       where;
    zend_string     *table;
    zend_array      *execute;
    zend_string     *sql;
    zend_string     *limit;
    zend_string     *order_by;           
    zend_object     std;
} lxx_database_t;

void lxx_database_instance(zval *this_ptr, zval *db_config);

LXX_MINIT_FUNCTION(database);

#endif