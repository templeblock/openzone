/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2014 Davorin Učakar
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
 * @file client/Profile.cc
 */

#include <client/Profile.hh>

#include <common/Lingua.hh>
#include <matrix/BotClass.hh>
#include <matrix/WeaponClass.hh>

#include <cwchar>
#include <cwctype>
#include <SDL.h>

namespace oz
{
namespace client
{

void Profile::save()
{
  File profileFile = config["dir.config"].get( String::EMPTY ) + "/profile.json";
  JSON profileConfig = JSON::OBJECT;

  profileConfig.add( "_version", OZ_VERSION );
  profileConfig.add( "name", name );
  profileConfig.add( "class", clazz == nullptr ? String::EMPTY : clazz->name );

  JSON& itemsConfig = profileConfig.add( "items", JSON::ARRAY );
  for( const ObjectClass* item : items ) {
    itemsConfig.add( item->name );
  }

  profileConfig.add( "weaponItem", weaponItem );
  profileConfig.add( "persistent", persistent );

  profileConfig.save( profileFile, CONFIG_FORMAT );
}

void Profile::init()
{
  File profileFile = config["dir.config"].get( String::EMPTY ) + "/profile.json";
  JSON profileConfig( profileFile );

  name       = profileConfig["name"].get( "" );
  clazz      = nullptr;
  weaponItem = -1;

  // Get username and capitalise it (needs conversion to Unicode and back to UTF-8).
  if( name.isEmpty() ) {
    const char* userName = SDL_getenv( "USER" );

    if( userName == nullptr || String::isEmpty( userName ) ) {
      name = OZ_GETTEXT( "Player" );
    }
    else {
      mbstate_t mbState;
      mSet( &mbState, 0, sizeof( mbState ) );

      const char* userNamePtr = userName;
      wchar_t wcUserName[128];
      mbsrtowcs( wcUserName, &userNamePtr, 128, &mbState );

      wcUserName[0] = wchar_t( towupper( wint_t( wcUserName[0] ) ) );

      mSet( &mbState, 0, sizeof( mbState ) );

      const wchar_t* wcUserNamePtr = wcUserName;
      char mbUserName[128];
      wcsrtombs( mbUserName, &wcUserNamePtr, 128, &mbState );

      name = mbUserName;
    }
  }

  if( profileConfig["_version"].get( String::EMPTY ) == OZ_VERSION ) {
    const String& sClazz = profileConfig["class"].get( String::EMPTY );
    clazz = sClazz.isEmpty() ? nullptr : static_cast<const BotClass*>( liber.objClass( sClazz ) );

    if( clazz != nullptr ) {
      const JSON& itemsConfig = profileConfig["items"];
      int nItems = itemsConfig.length();

      if( nItems > clazz->nItems ) {
        OZ_ERROR( "Too many items for player class '%s' in profile", clazz->name.cstr() );
      }

      items.clear();
      items.trim();

      for( int i = 0; i < nItems; ++i ) {
        const char* sItem = itemsConfig[i].get( "" );

        const ObjectClass* itemClazz = liber.objClass( sItem );
        if( ( itemClazz->flags & ( Object::DYNAMIC_BIT | Object::ITEM_BIT ) ) !=
            ( Object::DYNAMIC_BIT | Object::ITEM_BIT ) )
        {
          OZ_ERROR( "Invalid item '%s' in profile", sItem );
        }

        items.add( static_cast<const DynamicClass*>( itemClazz ) );
      }

      weaponItem = profileConfig["weaponItem"].get( -1 );

      if( weaponItem >= 0 ) {
        if( uint( weaponItem ) >= uint( items.length() ) ) {
          OZ_ERROR( "Invalid weaponItem #%d in profile", weaponItem );
        }

        const WeaponClass* weaponClazz = static_cast<const WeaponClass*>( items[weaponItem] );

        if( !( weaponClazz->flags & Object::WEAPON_BIT ) ) {
          OZ_ERROR( "Invalid weaponItem #%d '%s' in profile",
                    weaponItem, weaponClazz->name.cstr() );
        }

        if( !clazz->name.beginsWith( weaponClazz->userBase ) ) {
          OZ_ERROR( "Invalid weapon class '%s' for player class '%s' in profile",
                    weaponClazz->name.cstr(), clazz->name.cstr() );
        }
      }
    }
  }

  persistent = profileConfig["persistent"];

  if( persistent.type() != JSON::OBJECT ) {
    persistent = JSON::OBJECT;
  }
}

void Profile::destroy()
{
  save();

  items.clear();
  items.trim();
  persistent.clear();
}

Profile profile;

}
}
