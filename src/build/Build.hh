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
 * @file build/Build.hh
 *
 * Data builder.
 */

namespace oz
{
namespace build
{

class Build
{
  private:

    void printUsage();

    void copyFiles( const char* srcDir, const char* destDir, const char* ext, bool recurse );
    void buildTextures( const char* srcDir, const char* destDir, bool wrap,
                        int magFilter, int minFilter );

    void buildCaela();
    void buildTerrae();
    void buildBSPs();
    void buildBSPTextures();
    void tagClassResources();
    void tagFragResources();
    void buildModels();
    void copySounds();
    void buildModules();
    void checkLua( const char* path );
    void packArchive( const char* name );

  public:

    void shutdown();
    int  main( int argc, char** argv );

};

extern Build build;

}
}