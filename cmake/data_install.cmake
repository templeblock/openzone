#
# info files
#
if( WIN32 OR OZ_INSTALL_STANDALONE )
  file( GLOB files README.* )
  install( FILES
    COPYING
    README
    ${files}
    DESTINATION . COMPONENT info )
else()
  file( GLOB files README.* )
  install( FILES
    AUTHORS
    COPYING
    README
    ${files}
    DESTINATION share/doc COMPONENT info )
endif()

#
# icons
#
install( DIRECTORY share/applications share/pixmaps DESTINATION share COMPONENT icons )

#
# oalinst, DLLs
#
if( WIN32 )
  install( DIRECTORY support/mingw32-client/ DESTINATION bin COMPONENT client )
  install( FILES support/oalinst.exe support/openzone.bat DESTINATION . COMPONENT client )

  install( DIRECTORY support/mingw32-tools/ DESTINATION bin COMPONENT client-script )
  install( FILES support/prebuild.bat DESTINATION . COMPONENT build-script )
else()
  install( FILES support/openzone.sh COMPONENTS client-script PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION . )

  install( FILES support/build.sh COMPONENTS build-script PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION . )
endif()

#
# prebuilt data
#
install( FILES share/openzone/README DESTINATION share/openzone COMPONENT data )

# bsp
install( DIRECTORY share/openzone/bsp DESTINATION share/openzone COMPONENT data )

# caelum
install( DIRECTORY share/openzone/caelum DESTINATION share/openzone COMPONENT data
         FILES_MATCHING PATTERN "caelum/*.ozcCaelum" )

install( DIRECTORY share/openzone/caelum DESTINATION share/openzone COMPONENT data
         FILES_MATCHING PATTERN "caelum/*README*" )

install( DIRECTORY share/openzone/caelum DESTINATION share/openzone COMPONENT data
         FILES_MATCHING PATTERN "caelum/*COPYING*" )

# class
install( DIRECTORY share/openzone/class DESTINATION share/openzone COMPONENT data
         PATTERN "DISABLED" EXCLUDE )

# glsl
install( DIRECTORY share/openzone/glsl DESTINATION share/openzone COMPONENT data
         PATTERN "DISABLED" EXCLUDE )

# lua
install( DIRECTORY share/openzone/lua DESTINATION share/openzone COMPONENT data
         PATTERN "DISABLED" EXCLUDE )

# mdl
file( GLOB files
  share/openzone/mdl/*.ozcSMM
  share/openzone/mdl/*.ozcMD2
  share/openzone/mdl/*README*
  share/openzone/mdl/*COPYING* )

install( FILES ${files} DESTINATION share/openzone/mdl COMPONENT data )

# music
install( DIRECTORY share/openzone/music DESTINATION share/openzone COMPONENT data
         PATTERN "ALL" EXCLUDE )

# name
install( DIRECTORY share/openzone/name DESTINATION share/openzone COMPONENT data
         PATTERN "DISABLED" EXCLUDE )

# snd
install( DIRECTORY share/openzone/snd DESTINATION share/openzone COMPONENT data
         PATTERN "ALL" EXCLUDE )

# terra
install( DIRECTORY share/openzone/terra DESTINATION share/openzone COMPONENT data
         FILES_MATCHING PATTERN "terra/*.ozTerra" )

install( DIRECTORY share/openzone/terra DESTINATION share/openzone COMPONENT data
         FILES_MATCHING PATTERN "terra/*.ozcTerra" )

install( DIRECTORY share/openzone/terra DESTINATION share/openzone COMPONENT data
         FILES_MATCHING PATTERN "terra/*README*" )

install( DIRECTORY share/openzone/terra DESTINATION share/openzone COMPONENT data
         FILES_MATCHING PATTERN "terra/*COPYING*" )

# ui
install( DIRECTORY share/openzone/ui DESTINATION share/openzone COMPONENT data
         FILES_MATCHING PATTERN "ui/*/*.ozcTex" )

install( DIRECTORY share/openzone/ui DESTINATION share/openzone COMPONENT data
         FILES_MATCHING PATTERN "ui/*/*.ozcCur" )

install( DIRECTORY share/openzone/ui DESTINATION share/openzone COMPONENT data
         FILES_MATCHING PATTERN "ui/*/*.ttf" )

install( DIRECTORY share/openzone/ui DESTINATION share/openzone COMPONENT data
         FILES_MATCHING PATTERN "ui/*/*README*" )

install( DIRECTORY share/openzone/ui DESTINATION share/openzone COMPONENT data
         FILES_MATCHING PATTERN "ui/*/*COPYING*" )

# locale
install( DIRECTORY share/locale DESTINATION share COMPONENT data
         FILES_MATCHING PATTERN "*.mo" )

#
# development data
#
install( FILES share/openzone/README DESTINATION share/openzone COMPONENT data-build )

# caelum
install( DIRECTORY share/openzone/caelum DESTINATION share/openzone COMPONENT data-build
         PATTERN "*.ozcCaelum" EXCLUDE )

# class
install( DIRECTORY share/openzone/class DESTINATION share/openzone COMPONENT data-build
         PATTERN "DISABLED" EXCLUDE )

# data
install( DIRECTORY share/openzone/data/maps DESTINATION share/openzone/data COMPONENT data-build
         PATTERN "DISABLED" EXCLUDE
         PATTERN "*.bsp" EXCLUDE
         PATTERN "*.prt" EXCLUDE
         PATTERN "*.srf" EXCLUDE
         PATTERN "*autosave.map" EXCLUDE
         PATTERN "*.bak" EXCLUDE )

install( DIRECTORY share/openzone/data/scripts DESTINATION share/openzone/data COMPONENT data-build
         PATTERN "DISABLED" EXCLUDE )

install( DIRECTORY share/openzone/data/textures DESTINATION share/openzone/data COMPONENT data-build
         PATTERN "DISABLED" EXCLUDE )

# glsl
install( DIRECTORY share/openzone/glsl DESTINATION share/openzone COMPONENT data-build
         PATTERN "DISABLED" EXCLUDE )

# lua
install( DIRECTORY share/openzone/lua DESTINATION share/openzone COMPONENT data-build
         PATTERN "DISABLED" EXCLUDE )

# music
install( DIRECTORY share/openzone/music DESTINATION share/openzone COMPONENT data-build
         PATTERN "ALL" EXCLUDE )

# name
install( DIRECTORY share/openzone/name DESTINATION share/openzone COMPONENT data-build
         PATTERN "DISABLED" EXCLUDE )

# snd
install( DIRECTORY share/openzone/snd DESTINATION share/openzone COMPONENT data-build
         PATTERN "ALL" EXCLUDE )

# terra
install( DIRECTORY share/openzone/terra DESTINATION share/openzone COMPONENT data-build
         PATTERN "*.ozTerra" EXCLUDE
         PATTERN "*.ozcTerra" EXCLUDE )

# netradiant
install( DIRECTORY share/openzone/netradiant DESTINATION share/openzone COMPONENT data-build )

# mdl
install( DIRECTORY share/openzone/mdl DESTINATION share/openzone COMPONENT data-build
         PATTERN "DISABLED" EXCLUDE
         PATTERN "mdl/*.ozcSMM" EXCLUDE
         PATTERN "mdl/*.ozcMD2" EXCLUDE )

# ui
install( DIRECTORY share/openzone/ui DESTINATION share/openzone COMPONENT data-build
         PATTERN "DISABLED" EXCLUDE
         PATTERN "*.ozcCur" EXCLUDE
         PATTERN "*.ozcTex" EXCLUDE )

# locale
install( DIRECTORY share/locale DESTINATION share COMPONENT data-build )

# launcher, icon
install( DIRECTORY share/applications share/pixmaps DESTINATION share COMPONENT data-build )
