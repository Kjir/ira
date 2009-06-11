#! /bin/sh
# script shamelessy taken from openbox

sh() {
  /bin/sh -c "set -x; $*"
}

# We don't need those, but leave them if we do in the future
# sh autopoint --force # for GNU gettext
# sh libtoolize --copy --force --automake


sh aclocal -I m4 $ACLOCAL_FLAGS
headneeded=`grep -E "A(M|C)_CONFIG_HEADER" configure.[ia][nc]`
if test ! -z "$headneeded"; then sh autoheader; fi
sh autoconf
sh automake --add-missing --copy

echo
echo You are now ready to run ./configure
echo enjoy!
