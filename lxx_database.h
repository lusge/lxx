#ifndef LXX_DATABASE_H
#define LXX_DATABASE_H

typedef struct {
    smart_str       where;
    zend_string     *table;
    zend_array      *execute;
    zend_string     *sql;
    zend_string     *limit;
    zend_string     *order_by;
    zval            pdo;
    zend_string     *prefix;           
    zend_object     std;
} lxx_database_t;

LXX_MINIT_FUNCTION(database);

#endif