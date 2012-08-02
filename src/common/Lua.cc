/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2012 Davorin Učakar
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file common/Lua.cc
 */

#include "common/Lua.hh"

#include "common/luaapi.hh"

namespace oz
{

Lua::Lua() :
  l( null )
{}

bool Lua::readVariable( InputStream* istream )
{
  char ch = istream->readChar();

  switch( ch ) {
    case 'N': {
      l_pushnil();
      return true;
    }
    case 'b': {
      l_pushbool( istream->readBool() );
      return true;
    }
    case 'n': {
      l_pushdouble( istream->readDouble() );
      return true;
    }
    case 's': {
      l_pushstring( istream->readString() );
      return true;
    }
    case '[': {
      l_newtable();

      while( readVariable( istream ) ) { // key
        readVariable( istream ); // value

        l_rawset( -3 );
      }
      return true;
    }
    case ']': {
      return false;
    }
    default: {
      throw Exception( "Invalid type char '%c' in serialised Lua data", ch );
    }
  }
}

void Lua::writeVariable( BufferStream* ostream )
{
  int type = l_type( -1 );

  switch( type ) {
    case LUA_TNIL: {
      ostream->writeChar( 'N' );
      break;
    }
    case LUA_TBOOLEAN: {
      ostream->writeChar( 'b' );
      ostream->writeBool( l_tobool( -1 ) != 0 );
      break;
    }
    case LUA_TNUMBER: {
      ostream->writeChar( 'n' );
      ostream->writeDouble( l_todouble( -1 ) );
      break;
    }
    case LUA_TSTRING: {
      ostream->writeChar( 's' );
      ostream->writeString( l_tostring( -1 ) );
      break;
    }
    case LUA_TTABLE: {
      ostream->writeChar( '[' );

      l_pushnil();
      while( l_next( -2 ) != 0 ) {
        // key
        l_pushvalue( -2 );
        writeVariable( ostream );
        l_pop( 1 );

        // value
        writeVariable( ostream );

        l_pop( 1 );
      }

      ostream->writeChar( ']' );
      break;
    }
    default: {
      throw Exception( "Serialisation is only supported for LUA_TNIL, LUA_TBOOLEAN, LUA_TNUMBER,"
                       " LUA_TSTRING and LUA_TTABLE data types" );
    }
  }
}

void Lua::initCommon( const char* componentName )
{
  l = luaL_newstate();
  if( l == null ) {
    throw Exception( "Failed to create Lua state" );
  }

#if LUA_VERSION_NUM < 502
  lua_pushcfunction( l, luaopen_table );
  lua_pushcfunction( l, luaopen_string );
  lua_pushcfunction( l, luaopen_math );
  lua_pcall( l, 0, 0, 0 );
  lua_pcall( l, 0, 0, 0 );
  lua_pcall( l, 0, 0, 0 );
#else
  luaL_requiref( l, LUA_TABLIBNAME,  luaopen_table,  true );
  luaL_requiref( l, LUA_STRLIBNAME,  luaopen_string, true );
  luaL_requiref( l, LUA_MATHLIBNAME, luaopen_math,   true );
  lua_settop( l, 0 );
#endif

  if( l_gettop() != 0 ) {
    throw Exception( "Failed to initialise Lua libraries" );
  }

  ls.envName = componentName;

  IGNORE_FUNC( ozException );
  IGNORE_FUNC( ozPrintln );
}

void Lua::freeCommon()
{
  lua_close( l );
  l = null;
}

void Lua::registerFunction( const char* name, APIFunc func )
{
  l_register( name, func );
}

void Lua::registerConstant( const char* name, bool value )
{
  l_pushbool( value );
  l_setglobal( name );
}

void Lua::registerConstant( const char* name, int value )
{
  l_pushint( value );
  l_setglobal( name );
}

void Lua::registerConstant( const char* name, float value )
{
  l_pushfloat( value );
  l_setglobal( name );
}

void Lua::registerConstant( const char* name, const char* value )
{
  l_pushstring( value );
  l_setglobal( name );
}

}