AC_PREREQ(2.63)
AC_INIT([dspstream],[0.1])
AC_CONFIG_SRCDIR([src/main.cc])
AM_INIT_AUTOMAKE([-Wall])

AC_CANONICAL_HOST

AC_SEARCH_LIBS([strerror],[cposix])
AC_PROG_CXX
AC_LANG([C++])

AG_CHECK_IPP

if test "x$HAVE_IPP" = "xfalse"; then
	AC_MSG_ERROR(***Integrated Performance Primitives could not be found***)
else
	IPP_INCLUDES=-I$IPP_PREFIX/$ver/$IPP_CPU/include
    IPP_LDFLAGS=-L$IPP_PREFIX/$ver/$IPP_CPU/lib
	IPP_LIBS=-lippacemerged$IPP_SUFFIX -lippacmerged$IPP_SUFFIX -lippsemerged$IPP_SUFFIX -lippsmerged$IPP_SUFFIX -lippcore$IPP_SUFFIX
fi
AC_SUBST(IPP_INCLUDES)
AC_SUBST(IPP_LDFLAGS)
AC_SUBST(IPP_LIBS)

SHAVE_INIT(extra, enable)

AC_CONFIG_FILES([extra/shave Makefile test/Makefile])

AC_OUTPUT
