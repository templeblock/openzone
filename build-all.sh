#!/bin/sh

echo ================================================================
echo
echo                           Linux-x86_64
echo
echo ----------------------------------------------------------------

( cd Linux-x86_64 && time make -j4 )

echo ----------------------------------------------------------------
echo
echo                           Linux-x86_64
echo
echo ================================================================
echo
echo ================================================================
echo
echo                        Linux-x86_64-Clang
echo
echo ----------------------------------------------------------------

( cd Linux-x86_64-Clang && time make -j4 )

echo ----------------------------------------------------------------
echo
echo                        Linux-x86_64-Clang
echo
echo ================================================================
echo
echo ================================================================
echo
echo                            Linux-i686
echo
echo ----------------------------------------------------------------

( cd Linux-i686 && time make -j4 )

echo ----------------------------------------------------------------
echo
echo                            Linux-i686
echo
echo ================================================================
echo
echo ================================================================
echo
echo                         Linux-i686-Clang
echo
echo ----------------------------------------------------------------

( cd Linux-i686-Clang && time make -j4 )

echo ----------------------------------------------------------------
echo
echo                         Linux-i686-Clang
echo
echo ================================================================
echo
echo ================================================================
echo
echo                           Windows-i686
echo
echo ----------------------------------------------------------------

( cd Windows-i686 && time make -j4 )

echo ----------------------------------------------------------------
echo
echo                           Windows-i686
echo
echo ================================================================
#echo
#echo ================================================================
#echo
#echo                           NaCl-x86_64
#echo
#echo ----------------------------------------------------------------
#
#( cd NaCl-x86_64 && make -j4 )
#
#echo ----------------------------------------------------------------
#echo
#echo                           NaCl-x86_64
#echo
#echo ================================================================
echo
echo ================================================================
echo
echo                       NaCl-x86_64-newlib
echo
echo ----------------------------------------------------------------

( cd NaCl-x86_64-newlib && make -j4 )

echo ----------------------------------------------------------------
echo
echo                       NaCl-x86_64-newlib
echo
echo ================================================================
#echo
#echo ================================================================
#echo
#echo                            NaCl-i686
#echo
#echo ----------------------------------------------------------------
#
#( cd NaCl-i686 && make -j4 )
#
#echo ----------------------------------------------------------------
#echo
#echo                            NaCl-i686
#echo
#echo ================================================================
echo
echo ================================================================
echo
echo                         NaCl-i686-newlib
echo
echo ----------------------------------------------------------------

( cd NaCl-i686-newlib && make -j4 )

echo ----------------------------------------------------------------
echo
echo                         NaCl-i686-newlib
echo
echo ================================================================
