AC_PREREQ([2.59])
AC_INIT([dspstream], [0.1])
AC_CONFIG_SRCDIR([src/main.cc])
AM_INIT_AUTOMAKE([-Wall])

AC_ISC_POSIX
AC_PROG_CXX
AC_LANG_CPLUSPLUS

AG_CHECK_IPP

if test "x$HAVE_IPP" = "xfalse"; then
	AC_MSG_ERROR(***Integrated Performance Primitives could not be found***)
else
	IPP_INCLUDES="-I$IPP_PREFIX/$ver/$IPP_CPU/include"
	IPP_LIBS="-L$IPP_PREFIX/$ver/$IPP_CPU/lib -lippacemerged$IPP_SUFFIX -lippacmerged$IPP_SUFFIX -lippsemerged$IPP_SUFFIX -lippsmerged$IPP_SUFFIX -lippcore$IPP_SUFFIX"
fi
AC_SUBST(IPP_INCLUDES)
AC_SUBST(IPP_LIBS)

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
