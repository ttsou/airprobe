dnl
dnl Copyright 2004 Free Software Foundation, Inc.
dnl 
dnl This file is part of GNU Radio
dnl 
dnl GNU Radio is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2, or (at your option)
dnl any later version.
dnl 
dnl GNU Radio is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl 
dnl You should have received a copy of the GNU General Public License
dnl along with GNU Radio; see the file COPYING.  If not, write to
dnl the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
dnl Boston, MA 02111-1307, USA.
dnl

AC_DEFUN([GR_REQUIRE_BOOST_INCLUDES],
[
  AC_LANG_PUSH(C++)
  gr_boost_include_dir=
  AC_ARG_WITH([boost-include-dir],
    AC_HELP_STRING([--with-boost-include-dir=<path>],
	           [path to boost c++ include files]),
    [
      # "yes" and "no" are bogus answers
      if test x"$with_boost_include_dir" == xyes ||
         test x"$with_boost_include_dir" == xno; then
	gr_boost_include_dir=
      else
        gr_boost_include_dir=$with_boost_include_dir
      fi
    ])
  echo "gr_boost_include_dir = $gr_boost_include_dir"
  if test x$gr_boost_include_dir != x; then
    # if user specified a directory, then we use it
    OLD_CPPFLAGS=$CPPFLAGS
    CPPFLAGS="$CPPFLAGS -I$gr_boost_include_dir"
    AC_CHECK_HEADER([boost/shared_ptr.hpp],
      [BOOST_CFLAGS="-I$gr_boost_include_dir"],
      [AC_MSG_ERROR(
        [Failed to locate boost/shared_ptr.hpp.
Try using --with-boost-include-dir=<path>])])
    CPPFLAGS=$OLD_CPPFLAGS
  else
    # is the header in the default place?
    AC_CHECK_HEADER([boost/shared_ptr.hpp],
      [BOOST_CFLAGS=""],
      [ # nope, look one more place
	# wipe out cached value.  KLUDGE: AC should have API for this
	unset AS_TR_SH([ac_cv_header_boost/shared_ptr.hpp])
        p=/usr/local/include/boost-1_31
        OLD_CPPFLAGS=$CPP_FLAGS
        CPPFLAGS="$CPPFLAGS -I$p"
        AC_CHECK_HEADER([boost/shared_ptr.hpp],
          [BOOST_CFLAGS="-I$p"],
          [AC_MSG_ERROR(
            [Failed to locate boost/shared_ptr.hpp.  
Try using --with-boost-include-dir=<path>])])
        CPPFLAGS=$OLD_CPPFLAGS
      ])

   fi
   AC_LANG_POP
   AC_SUBST(BOOST_CFLAGS)
])
