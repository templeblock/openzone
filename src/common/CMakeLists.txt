add_library(common STATIC
#BEGIN SOURCES
  AABB.hh
  Bounds.hh
  common.hh
  Lingua.hh
  luaapi.hh
  luabase.hh
  Timer.hh
  Lingua.cc
  luabase.cc
  Timer.cc
#END SOURCES
)
use_pch(common pch)
