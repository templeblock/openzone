#!/bin/sh

buildType=Debug
platforms=( \
  Linux-x86_64 Linux-x86_64-Clang Linux-i686 Linux-i686-Clang \
  Windows-i686 \
  NaCl-x86_64 NaCl-i686 \
#   Android9-i686 Android9-ARM Android9-MIPS \
)

function clean()
{
  for platform in ${platforms[@]}; do
    rm -rf build/$platform
  done
}

function build()
{
  for platform in ${platforms[@]}; do
    echo ================================================================
    echo
    echo                           $platform
    echo
    echo ----------------------------------------------------------------

    (( $1 )) && rm -rf build/$platform
    if [[ ! -d build/$platform ]]; then
      mkdir -p build/$platform
      ( cd build/$platform && cmake ../.. \
        -DCMAKE_BUILD_TYPE=$buildType \
        -DCMAKE_TOOLCHAIN_FILE=../../cmake/$platform.Toolchain.cmake )
    fi
    (( $1 )) || ( cd build/$platform && time make -j4 )

    echo ----------------------------------------------------------------
    echo
    echo                           $platform
    echo
    echo ================================================================
    echo
  done
}

case "$1" in
  clean)
    clean
    ;;
  conf)
    build 1
    ;;
  *)
    build 0
    ;;
esac