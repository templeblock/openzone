/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2013 Davorin Učakar
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

#include <stable.hh>
#include <builder/Lingua.hh>

namespace oz
{
namespace builder
{

void Lingua::buildCatalogue( const char* directory, const char* catalogue )
{
  Log::print( "%s/%s ...", directory, catalogue );

  File::mkdir( directory );

  File srcFile( File::VIRTUAL, String::str( "%s/%s.po", directory, catalogue ) );
  File outFile( File::NATIVE, String::str( "%s/%s.ozCat", directory, catalogue ) );

  if( srcFile.type() != File::REGULAR ) {
    OZ_ERROR( "Cannot read catalogue source file '%s'", srcFile.path().cstr() );
  }

  Buffer buffer = srcFile.read();
  InputStream is = buffer.inputStream();

  List<String> messages;

  String lastOriginal;
  String lastTranslation;

  int mode = 0; // 1 for original, 2 for translation
  int lineNum = 0;

  while( is.isAvailable() ) {
    ++lineNum;

    String line = is.readLine();

    // replace "\n" with the real newline
    for( int i = 1; i < line.length(); ++i ) {
      if( line[i] == '\0' ) {
        break;
      }
      if( line[i - 1] == '\\' ) {
        switch( line[i] ) {
          case '\\':
          case '"': {
            line = line.substring( 0, i - 1 ) + line.substring( i );
            break;
          }
          case 'n': {
            line = line.substring( 0, i - 1 ) + "\n" + line.substring( i + 1 );
            break;
          }
          default: {
            OZ_ERROR( "%s:%d: invalid escape sequence '\\%c'",
                      srcFile.path().cstr(), lineNum, line[i] );
          }
        }
      }
    }

    if( line.isEmpty() || line[0] == '\n' || line[0] == '#' ) {
      continue;
    }

    if( String::beginsWith( line, "msgid" ) ) {
      mode = 1;

      if( !lastOriginal.isEmpty() && !lastOriginal.equals( lastTranslation ) ) {
        messages.add( lastOriginal );
        messages.add( lastTranslation );
      }

      int begin = line.index( '"' );
      int end   = line.lastIndex( '"' );

      if( begin < 0 || end < 0 || begin >= end ) {
        OZ_ERROR( "%s:%d: syntax error", srcFile.path().cstr(), lineNum );
      }

      lastOriginal = line.substring( begin + 1, end );
      lastTranslation = "";
    }

    if( String::beginsWith( line, "msgstr" ) ) {
      mode = 2;

      int begin = line.index( '"' );
      int end   = line.lastIndex( '"' );

      if( begin < 0 || end < 0 || begin >= end ) {
        OZ_ERROR( "%s:%d: syntax error", srcFile.path().cstr(), lineNum );
      }

      lastTranslation = line.substring( begin + 1, end );
    }

    if( line[0] == '"' ) {
      int begin = line.index( '"' );
      int end   = line.lastIndex( '"' );

      if( begin < 0 || end < 0 || begin >= end ) {
        OZ_ERROR( "%s:%d: syntax error", srcFile.path().cstr(), lineNum );
      }

      if( mode == 1 ) {
        lastOriginal = lastOriginal + line.substring( begin + 1, end );
      }
      else if( mode == 2 ) {
        lastTranslation = lastTranslation + line.substring( begin + 1, end );
      }
      else {
        OZ_ERROR( "%s:%d: loose string", srcFile.path().cstr(), lineNum );
      }
    }
  }

  if( !lastOriginal.isEmpty() && !lastOriginal.equals( lastTranslation ) ) {
    messages.add( lastOriginal );
    messages.add( lastTranslation );
  }

  OutputStream ostream( 0 );

  ostream.writeInt( messages.length() / 2 );

  for( int i = 0; i < messages.length(); i += 2 ) {
    ostream.writeInt( hash( messages[i] ) );
    ostream.writeString( messages[i] );
    ostream.writeString( messages[i + 1] );
  }

  if( !outFile.write( ostream.begin(), ostream.length() ) ) {
    OZ_ERROR( "Catalogue write failed" );
  }

  Log::printEnd( " OK" );
}

void Lingua::build()
{
  Log::println( "Building package localisations {" );
  Log::indent();

  File linguaDir( File::VIRTUAL, "lingua" );
  DArray<File> languages = linguaDir.ls();

  if( !languages.isEmpty() ) {
    File::mkdir( linguaDir.path() );
  }

  foreach( langDir, languages.iter() ) {
    if( langDir->type() != File::DIRECTORY ) {
      continue;
    }

    DArray<File> catalogues = langDir->ls();

    if( !catalogues.isEmpty() ) {
      File::mkdir( langDir->path() );
    }

    foreach( catalogue, catalogues.citer() ) {
      if( !catalogue->hasExtension( "po" ) ) {
        continue;
      }

      buildCatalogue( langDir->path(), catalogue->baseName() );
    }
  }

  File missionsDir( File::VIRTUAL, "mission" );
  DArray<File> missions = missionsDir.ls();

  foreach( mission, missions.citer() ) {
    linguaDir = File( File::VIRTUAL, mission->path() + "/lingua" );
    languages = linguaDir.ls();

    foreach( catalogue, languages.citer() ) {
      if( !catalogue->hasExtension( "po" ) ) {
        continue;
      }

      File::mkdir( "mission" );
      File::mkdir( mission->path() );
      File::mkdir( linguaDir.path() );

      buildCatalogue( linguaDir.path(), catalogue->baseName() );
    }
  }

  Log::unindent();
  Log::println( "}" );
}

void Lingua::buildMissions()
{
  Log::println( "Building mission localisations {" );
  Log::indent();

  File missionsDir( File::VIRTUAL, "mission" );
  DArray<File> missions = missionsDir.ls();

  foreach( mission, missions.citer() ) {
    File linguaDir( File::VIRTUAL, mission->path() + "/lingua" );
    DArray<File> languages = linguaDir.ls();

    foreach( catalogue, languages.citer() ) {
      if( !catalogue->hasExtension( "po" ) ) {
        continue;
      }

      File::mkdir( "mission" );
      File::mkdir( mission->path() );
      File::mkdir( linguaDir.path() );

      buildCatalogue( linguaDir.path(), catalogue->baseName() );
    }
  }

  Log::unindent();
  Log::println( "}" );
}

Lingua lingua;

}
}
