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
 * @file client/GameStage.cc
 */

#include "stable.hh"

#include "client/GameStage.hh"

#include "matrix/Bot.hh"
#include "matrix/Synapse.hh"
#include "matrix/Matrix.hh"

#include "nirvana/Nirvana.hh"

#include "modules/Modules.hh"

#include "client/Loader.hh"
#include "client/Render.hh"
#include "client/Sound.hh"
#include "client/Network.hh"
#include "client/Camera.hh"
#include "client/Lua.hh"
#include "client/QuestList.hh"
#include "client/MenuStage.hh"

#include "client/ui/LoadingArea.hh"

namespace oz
{
namespace client
{

using oz::matrix::matrix;
using oz::nirvana::nirvana;

GameStage gameStage;

String GameStage::AUTOSAVE_FILE;
String GameStage::QUICKSAVE_FILE;

int GameStage::auxMain( void* )
{
  try {
    gameStage.auxRun();
  }
  catch( const std::exception& e ) {
    oz::log.verboseMode = false;
    oz::log.printException( &e );

    oz::System::bell();
    oz::System::abort();
  }
  return 0;
}

bool GameStage::read( const char* path )
{
  log.print( "Loading state from '%s' ...", path );

  File file( path );
  if( !file.map() ) {
    log.printEnd( " Failed" );
    return false;
  }

  log.printEnd( " OK" );

  InputStream istream = file.inputStream();

  matrix.read( &istream );
  nirvana.read( &istream );

  log.println( "Reading Client {" );
  log.indent();

  questList.read( &istream );
  camera.read( &istream );
  modules.read( &istream );

  lua.read( &istream );

  log.unindent();
  log.println( "}" );

  file.unmap();

  return true;
}

void GameStage::write( const char* path ) const
{
  BufferStream ostream;

  matrix.write( &ostream );
  nirvana.write( &ostream );

  log.print( "Writing Client ..." );

  questList.write( &ostream );
  camera.write( &ostream );
  modules.write( &ostream );

  lua.write( &ostream );

  log.printEnd( " OK" );

  log.print( "Saving state to %s ...", path );

  if( !File( path ).write( &ostream ) ) {
    log.printEnd( " Failed" );
  }
  else {
    log.printEnd( " OK" );
  }
}

void GameStage::reload()
{
  log.print( "[" );
  log.printTime();
  log.printEnd( "] Reloading GameStage {" );
  log.indent();

  ui::mouse.doShow = false;
  ui::ui.loadingScreen->status.setText( "%s", OZ_GETTEXT( "Loading ..." ) );
  ui::ui.showLoadingScreen( true );
  ui::ui.root->focus( ui::ui.loadingScreen );

  render.draw( Render::DRAW_UI_BIT );
  render.draw( Render::DRAW_UI_BIT );
  render.swap();

  camera.reset();

  modules.unload();
  context.unload();
  render.unload();
  questList.unload();

  lua.free();

  nirvana.unload();
  matrix.unload();

  matrix.load();
  nirvana.load();

  lua.init();
  modules.registerLua();

  questList.load();
  render.load();
  context.load();
  modules.load();

  if( stateFile.isEmpty() ) {
    log.println( "Initialising new world" );

    lua.create( mission );

    if( orbis.terra.id == -1 || orbis.caelum.id == -1 ) {
      throw Exception( "Terrain and Caelum must both be loaded via the client.onCreate" );
    }
  }
  else {
    if( !read( stateFile ) ) {
      throw Exception( "Reading saved state '%s' failed", stateFile.cstr() );
    }
  }

  nirvana.sync();
  synapse.update();

  camera.update();
  camera.prepare();

  render.draw( Render::DRAW_ORBIS_BIT | Render::DRAW_UI_BIT );
  sound.play();
  render.swap();

  loader.loadScheduled();

  ui::ui.prepare();
  ui::ui.showLoadingScreen( false );

  log.unindent();
  log.println( "}" );
}

void GameStage::auxRun()
{
  uint beginTime;

  SDL_SemPost( mainSemaphore );
  SDL_SemPost( mainSemaphore );
  SDL_SemWait( auxSemaphore );

  while( isAuxAlive ) {
    /*
     * PHASE 2
     */
    beginTime = Time::clock();

    network.update();

    // update world
    matrix.update();

    matrixMillis += Time::clock() - beginTime;

    SDL_SemPost( mainSemaphore );
    SDL_SemWait( auxSemaphore );

    /*
     * PHASE 3
     */
    beginTime = Time::clock();

    // sync nirvana
    nirvana.sync();

    // now synapse lists are not needed any more
    synapse.update();

    // update minds
    nirvana.update();

    nirvanaMillis += Time::clock() - beginTime;

    // we can now manipulate world from the main thread after synapse lists have been cleared
    // and nirvana is not accessing matrix any more
    SDL_SemPost( mainSemaphore );

    /*
     * PHASE 1
     */

    SDL_SemPost( mainSemaphore );
    SDL_SemWait( auxSemaphore );
  }
}

bool GameStage::update()
{
  uint beginTime;

  SDL_SemWait( mainSemaphore );

  /*
   * PHASE 1
   */

  beginTime = Time::clock();

  if( ui::keyboard.keys[SDLK_F5] && !ui::keyboard.oldKeys[SDLK_F5] ) {
    write( QUICKSAVE_FILE );
  }
  if( ui::keyboard.keys[SDLK_F7] && !ui::keyboard.oldKeys[SDLK_F7] ) {
    stateFile = QUICKSAVE_FILE;
    reload();
  }
  if( ui::keyboard.keys[SDLK_F8] && !ui::keyboard.oldKeys[SDLK_F8] ) {
    stateFile = AUTOSAVE_FILE;
    reload();
  }
  if( ui::keyboard.keys[SDLK_ESCAPE] ) {
    Stage::nextStage = &menuStage;
  }

  camera.update();
  ui::ui.update();
  modules.update();
  lua.update();

  uiMillis += Time::clock() - beginTime;

  SDL_SemPost( auxSemaphore );
  SDL_SemWait( mainSemaphore );

  /*
   * PHASE 2
   */

  beginTime = Time::clock();

  context.updateLoad();

  loader.cleanup();
  loader.loadScheduled();

  loaderMillis += Time::clock() - beginTime;

  SDL_SemPost( auxSemaphore );
  SDL_SemWait( mainSemaphore );

  /*
   * PHASE 3
   */

  camera.prepare();

  return true;
}

void GameStage::present( bool full )
{
  uint beginTime = Time::clock();
  uint currentTime;

  sound.play();

  currentTime = Time::clock();
  presentMillis += currentTime - beginTime;

  if( full ) {
    render.draw( Render::DRAW_ORBIS_BIT | Render::DRAW_UI_BIT );
    render.swap();
  }
}

void GameStage::load()
{
  log.print( "[" );
  log.printTime();
  log.printEnd( "] Loading GameStage {" );
  log.indent();

  loadingMillis = Time::clock();

  ui::mouse.doShow = false;
  ui::ui.loadingScreen->status.setText( "%s", OZ_GETTEXT( "Loading ..." ) );
  ui::ui.loadingScreen->show( true );

  render.draw( Render::DRAW_UI_BIT );
  render.draw( Render::DRAW_UI_BIT );
  render.swap();

  timer.reset();

  uiMillis      = 0;
  loaderMillis  = 0;
  presentMillis = 0;
  renderMillis  = 0;
  matrixMillis  = 0;
  nirvanaMillis = 0;

  log.print( "Starting auxilary thread ..." );

  isAuxAlive    = true;
  mainSemaphore = SDL_CreateSemaphore( 0 );
  auxSemaphore  = SDL_CreateSemaphore( 0 );
  auxThread     = SDL_CreateThread( auxMain, null );

  log.printEnd( " OK" );

  network.connect();

  matrix.load();
  nirvana.load();

  lua.init();
  modules.registerLua();

  questList.load();
  render.load();
  context.load();

  camera.reset();
  camera.setState( Camera::STRATEGIC );

  modules.load();

  if( stateFile.isEmpty() ) {
    log.println( "Initialising new world" );

    log.println( "Loading Client {" );
    log.indent();

    lua.create( mission );

    log.unindent();
    log.println( "}" );

    if( orbis.terra.id == -1 || orbis.caelum.id == -1 ) {
      throw Exception( "Terrain and Caelum must both be loaded via the client.onCreate" );
    }
  }
  else if( !read( stateFile ) ) {
    throw Exception( "Reading saved state '%s' failed", stateFile.cstr() );
  }

  nirvana.sync();
  synapse.update();

  ui::mouse.buttons = 0;
  ui::mouse.currButtons = 0;

  camera.update();
  camera.prepare();

  ui::ui.showLoadingScreen( true );

  render.draw( Render::DRAW_ORBIS_BIT | Render::DRAW_UI_BIT );
  sound.play();
  render.swap();

  loader.loadScheduled();

  ui::ui.prepare();
  ui::ui.showLoadingScreen( false );

  loadingMillis = Time::clock() - loadingMillis;

  isLoaded = true;

  log.unindent();
  log.println( "}" );
}

void GameStage::unload()
{
  log.print( "[" );
  log.printTime();
  log.printEnd( "] Unloading GameStage {" );
  log.indent();

  ui::mouse.doShow = false;
  ui::ui.loadingScreen->status.setText( "%s", OZ_GETTEXT( "Shutting down ..." ) );
  ui::ui.showLoadingScreen( true );

  render.draw( Render::DRAW_UI_BIT );
  render.swap();

  float uiTime                = float( uiMillis )                       * 0.001f;
  float loaderTime            = float( loaderMillis )                   * 0.001f;
  float presentTime           = float( presentMillis )                  * 0.001f;
  float renderTime            = float( renderMillis )                   * 0.001f;
  float renderPrepareTime     = float( render.prepareMillis )           * 0.001f;
  float renderSetupTime       = float( render.setupMillis )             * 0.001f;
  float renderCaelumTime      = float( render.caelumMillis )            * 0.001f;
  float renderTerraTime       = float( render.terraMillis )             * 0.001f;
  float renderStructsTime     = float( render.structsMillis )           * 0.001f;
  float renderObjectsTime     = float( render.objectsMillis )           * 0.001f;
  float renderFragsTime       = float( render.fragsMillis )             * 0.001f;
  float renderMiscTime        = float( render.miscMillis )              * 0.001f;
  float renderPostprocessTime = float( render.postprocessMillis )       * 0.001f;
  float renderUITime          = float( render.uiMillis )                * 0.001f;
  float renderSwapTime        = float( render.swapMillis )              * 0.001f;
  float matrixTime            = float( matrixMillis )                   * 0.001f;
  float nirvanaTime           = float( nirvanaMillis )                  * 0.001f;
  float loadingTime           = float( loadingMillis )                  * 0.001f;
  float runTime               = float( timer.runMicros )                * 1.0e-6f;
  float gameTime              = float( timer.micros )                   * 1.0e-6f;
  float droppedTime           = float( timer.runMicros - timer.micros ) * 1.0e-6f;
  float frameDropRate         = float( timer.ticks - timer.nFrames ) / float( timer.ticks );

  if( isLoaded ) {
    write( AUTOSAVE_FILE );
  }

  modules.unload();

  camera.reset();

  context.unload();
  render.unload();
  questList.unload();

  lua.free();

  nirvana.unload();
  matrix.unload();

  network.disconnect();

  log.print( "Stopping auxilary thread ..." );

  isAuxAlive = false;

  SDL_SemPost( auxSemaphore );
  SDL_WaitThread( auxThread, null );

  SDL_DestroySemaphore( auxSemaphore );
  SDL_DestroySemaphore( mainSemaphore );

  mainSemaphore = null;
  auxSemaphore  = null;
  auxThread     = null;

  log.printEnd( " OK" );

  ui::ui.showLoadingScreen( false );

  log.println( "Time statistics {" );
  log.indent();
  log.println( "loading time          %8.2f s",         loadingTime                              );
  log.println( "run time              %8.2f s",         runTime                                  );
  log.println( "game time             %8.2f s  ",       gameTime                                 );
  log.println( "dropped time          %8.2f s",         droppedTime                              );
  log.println( "optimal tick/frame rate %6.2f Hz ",     1.0f / Timer::TICK_TIME                  );
  log.println( "tick rate in run time   %6.2f Hz ",     float( timer.ticks ) / runTime           );
  log.println( "frame rate in run time  %6.2f Hz",      float( timer.nFrames ) / runTime         );
  log.println( "frame drop              %6.2f %%",      frameDropRate * 100.0f                   );
  log.println( "Run time usage {" );
  log.indent();
  log.println( "%6.2f %%  [M:1] input & ui",            uiTime                / runTime * 100.0f );
  log.println( "%6.2f %%  [M:2] loader",                loaderTime            / runTime * 100.0f );
  log.println( "%6.2f %%  [M:3] present",               presentTime           / runTime * 100.0f );
  log.println( "%6.2f %%  [M:3] render",                renderTime            / runTime * 100.0f );
  log.println( "%6.2f %%  [M:3] + render prepare",      renderPrepareTime     / runTime * 100.0f );
  log.println( "%6.2f %%  [M:3] + render shader setup", renderSetupTime       / runTime * 100.0f );
  log.println( "%6.2f %%  [M:3] + render caelum",       renderCaelumTime      / runTime * 100.0f );
  log.println( "%6.2f %%  [M:3] + render terra",        renderTerraTime       / runTime * 100.0f );
  log.println( "%6.2f %%  [M:3] + render structs",      renderStructsTime     / runTime * 100.0f );
  log.println( "%6.2f %%  [M:3] + render objects",      renderObjectsTime     / runTime * 100.0f );
  log.println( "%6.2f %%  [M:3] + render frags",        renderFragsTime       / runTime * 100.0f );
  log.println( "%6.2f %%  [M:3] + render misc",         renderMiscTime        / runTime * 100.0f );
  log.println( "%6.2f %%  [M:3] + render postprocess",  renderPostprocessTime / runTime * 100.0f );
  log.println( "%6.2f %%  [M:3] + render ui",           renderUITime          / runTime * 100.0f );
  log.println( "%6.2f %%  [M:3] + render swap",         renderSwapTime        / runTime * 100.0f );
  log.println( "%6.2f %%  [A:2] matrix",                matrixTime            / runTime * 100.0f );
  log.println( "%6.2f %%  [A:3] nirvana",               nirvanaTime           / runTime * 100.0f );
  log.unindent();
  log.println( "}" );
  log.unindent();
  log.println( "}" );

  isLoaded = false;

  log.unindent();
  log.println( "}" );
}

void GameStage::init()
{
  isLoaded = false;

  log.println( "Initialising GameStage {" );
  log.indent();

  AUTOSAVE_FILE = String::str( "%s/saves/autosave.ozState", config.get( "dir.config", "" ) );
  QUICKSAVE_FILE = String::str( "%s/saves/quicksave.ozState", config.get( "dir.config", "" ) );

  matrix.init();
  nirvana.init();
  loader.init();
  modules.init();

  log.unindent();
  log.println( "}" );
}

void GameStage::free()
{
  log.println( "Freeing GameStage {" );
  log.indent();

  modules.free();
  loader.free();
  nirvana.free();
  matrix.free();

  stateFile      = "";
  mission        = "";
  AUTOSAVE_FILE  = "";
  QUICKSAVE_FILE = "";

  log.unindent();
  log.println( "}" );
}

}
}
