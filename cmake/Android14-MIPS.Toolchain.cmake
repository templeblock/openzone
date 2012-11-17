set( ANDROID ON )

set( PLATFORM_TRIPLET           "mipsel-linux-android" )
set( PLATFORM_PROC_PREFIX       "mips" )
set( PLATFORM_PREFIX            "/opt/android-ndk/platforms/android-14/arch-mips" )
set( PLATFORM_TOOL_PREFIX       "/opt/android-ndk/toolchains/mipsel-linux-android-4.6/prebuilt/linux-x86" )
set( PLATFORM_STL_PREFIX        "/opt/android-ndk/sources/cxx-stl/gnu-libstdc++/4.6" )
set( PLATFORM_PORTS_PREFIX      "${CMAKE_SOURCE_DIR}/ports/Android14-MIPS" )
set( PLATFORM_FLAGS             "-fPIC --sysroot=${PLATFORM_PREFIX}" )
set( PLATFORM_FLAGS             "${PLATFORM_FLAGS} -isysroot ${PLATFORM_STL_PREFIX}/include" )
set( PLATFORM_FLAGS             "${PLATFORM_FLAGS} -isysroot ${PLATFORM_STL_PREFIX}/libs/mips/include" )
set( PLATFORM_STL_LIBRARY       "${PLATFORM_STL_PREFIX}/libs/mips/libgnustl_shared.so" )

set( CMAKE_SYSTEM_NAME          "Linux" CACHE STRING "Target system." )
set( CMAKE_SYSTEM_PROCESSOR     "MIPS" CACHE STRING "Target processor." )
set( CMAKE_FIND_ROOT_PATH       "${PLATFORM_PORTS_PREFIX};${PLATFORM_PREFIX}" )
set( CMAKE_C_COMPILER           "${PLATFORM_TOOL_PREFIX}/bin/${PLATFORM_TRIPLET}-gcc" )
set( CMAKE_CXX_COMPILER         "${PLATFORM_TOOL_PREFIX}/bin/${PLATFORM_TRIPLET}-g++" )
set( CMAKE_C_FLAGS              "${PLATFORM_FLAGS}" CACHE STRING "" )
set( CMAKE_CXX_FLAGS            "${PLATFORM_FLAGS}" CACHE STRING "" )
