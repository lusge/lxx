#ifndef GODA_EXCEPTION_H
#define GODA_EXCEPTION_H

extern zend_class_entry *lxx_exception_ce;

void lxx_throw_exception(int type, const char *format, ...);

LXX_MINIT_FUNCTION(exception);

#endif