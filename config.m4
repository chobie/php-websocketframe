PHP_ARG_ENABLE(websocketframe, Whether to enable the "websocketframe" extension,
    [ --enable-websocketframe  Enable "websocketframe" extension support])

PHP_ARG_ENABLE(websocketframe-debug, for websocketframe debug support,
    [ --enable-websocketframe-debug       Enable enable websocketframe debug support], no, no)

if test $PHP_WEBSOCKETFRAME != "no"; then
    if test "$PHP_WEBSOCKETFRAME_DEBUG" != "no"; then
        CFLAGS="$CFLAGS -Wall -g -ggdb -O0 -DPHP_WEBSOCKETFRAME_DEBUG=1"
        AC_DEFINE($PHP_WEBSOCKETFRAME_DEBUG, 1, [Enable websocketframe debug support])
    else
        //CFLAGS="$CFLAGS -Wall -g -ggdb -O0 -Wunused-variable -Wpointer-sign -Wimplicit-function-declaration -Winline -Wunused-macros -Wredundant-decls -Wstrict-aliasing=2 -Wswitch-enum -Wdeclaration-after-statement"
    fi

    PHP_WEBSOCKETFRAME_SRCS="websocketframe.c"

    PHP_SUBST(WEBSOCKETFRAME_SHARED_LIBADD)
    PHP_NEW_EXTENSION(websocketframe, $PHP_WEBSOCKETFRAME_SRCS, $ext_shared)
    PHP_ADD_EXTENSION_DEP(websocketframe, spl, true)

    PHP_SUBST([CFLAGS])
fi
