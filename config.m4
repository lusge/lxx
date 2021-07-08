dnl config.m4 for extension lxx

PHP_ARG_ENABLE(lxx, whether to enable lxx support,
dnl Make sure that the comment is aligned:
[  --enable-lxx          Enable lxx support], no)

if test "$PHP_LXX" != "no"; then
  AC_DEFINE(HAVE_LXX, 1, [ Have lxx support ])

  PHP_NEW_EXTENSION(lxx, 
  lxx.c \
  rax/rax.c \
  lxx_router.c  \
  tools/lxx_radix_tree.c  \
  lxx_request.c \
  lxx_application.c \
  lxx_controller.c  \
  lxx_response.c  \
  lxx_loader.c  \
  lxx_config.c  \
  lxx_exception.c, 
  $ext_shared)
fi
