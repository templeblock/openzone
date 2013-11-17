/*
 * ozEngine - OpenZone Engine Library.
 *
 * Copyright © 2002-2013 Davorin Učakar
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgement in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/**
 * @file ozEngine/Window.cc
 */

#include "Window.hh"

#include "Pepper.hh"
#include "GL.hh"

#include <SDL.h>
#include <png.h>

#ifdef __native_client__
# include <ppapi/cpp/completion_callback.h>
# include <ppapi/cpp/instance.h>
# include <ppapi/cpp/graphics_3d.h>
# include <ppapi/gles2/gl2ext_ppapi.h>
#endif

namespace oz
{

struct ScreenshotInfo
{
  File  file;
  int   width;
  int   height;
  char* pixels;
};

#if defined( __native_client__ )
static Semaphore       flushSemaphore;
static pp::Graphics3D* context;
#elif SDL_MAJOR_VERSION < 2
static SDL_Surface*    descriptor;
#else
static SDL_Window*     descriptor;
static SDL_GLContext   context;
#endif
static Thread          screenshotThread;

static void writeFunc( png_struct* png, ubyte* data, size_t size )
{
  OutputStream* os = static_cast<OutputStream*>( png_get_io_ptr( png ) );

  os->writeChars( reinterpret_cast<const char*>( data ), int( size ) );
}

static void flushFunc( png_struct* )
{}

static void screenshotMain( void* data )
{
  const ScreenshotInfo* info = static_cast<const ScreenshotInfo*>( data );

  OutputStream os( 0 );

  png_struct* png     = png_create_write_struct( PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr );
  png_info*   pngInfo = png_create_info_struct( png );

  png_set_IHDR( png, pngInfo, uint( info->width ), uint( info->height ), 8, PNG_COLOR_TYPE_RGB,
                PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );

  png_set_write_fn( png, &os, writeFunc, flushFunc );
  png_write_info( png, pngInfo );

  int pitch = ( ( info->width * 3 + 3 ) / 4 ) * 4;

  for( int i = info->height - 1; i >= 0; --i ) {
    const char* row = &info->pixels[i * pitch];

    png_write_row( png, reinterpret_cast<const ubyte*>( row ) );
  }

  png_write_end( png, pngInfo );
  png_destroy_write_struct( &png, &pngInfo );

  info->file.write( os.begin(), os.tell() );

  delete[] info->pixels;
  delete info;
}

#ifdef __native_client__

static void flushCompleteCallback( void*, int )
{
  flushSemaphore.post();
}

#endif

int  Window::screenWidth  = 0;
int  Window::screenHeight = 0;
int  Window::windowWidth  = 0;
int  Window::windowHeight = 0;
bool Window::fullscreen   = false;
bool Window::windowFocus  = true;
bool Window::windowGrab   = false;

void Window::measureScreen()
{
#if defined( __native_client__ )

  screenWidth  = Pepper::width;
  screenHeight = Pepper::height;

#elif SDL_MAJOR_VERSION < 2

  const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();

  screenWidth  = videoInfo->current_w;
  screenHeight = videoInfo->current_h;

#else

  SDL_DisplayMode mode;
  SDL_GetDesktopDisplayMode( 0, &mode );

  screenWidth  = mode.w;
  screenHeight = mode.h;

#endif
}

bool Window::isCreated()
{
#ifdef __native_client__
  return !context->is_null();
#else
  return descriptor != nullptr;
#endif
}

void Window::setGrab( bool grab )
{
  windowGrab = grab;

#if SDL_MAJOR_VERSION < 2
  SDL_ShowCursor( !windowGrab );
  SDL_WM_GrabInput( SDL_GrabMode( windowGrab && !System::isInstrumented() ) );
#else
  SDL_SetRelativeMouseMode( SDL_bool( windowGrab ) );
#endif
}

void Window::warpMouse()
{
  if( !windowFocus || !windowGrab ) {
    return;
  }

#ifdef __native_client__

  Pepper::moveX = 0.0f;
  Pepper::moveY = 0.0f;
  Pepper::moveZ = 0.0f;
  Pepper::moveW = 0.0f;

#elif SDL_MAJOR_VERSION >= 2

  SDL_WarpMouseInWindow( descriptor, windowWidth / 2, windowHeight / 2 );
  SDL_PumpEvents();
  SDL_GetRelativeMouseState( nullptr, nullptr );

#endif
}

void Window::swapBuffers()
{
#if defined( __native_client__ )

  MainCall() << []()
  {
    context->SwapBuffers( pp::CompletionCallback( flushCompleteCallback, nullptr ) );
  };
  flushSemaphore.wait();

#elif SDL_MAJOR_VERSION < 2

  SDL_GL_SwapBuffers();

#else

  SDL_GL_SwapWindow( descriptor );

#endif
}

void Window::screenshot( const File& file )
{
  if( screenshotThread.isValid() ) {
    screenshotThread.join();
  }

  int   pitch  = ( ( windowWidth * 3 + 3 ) / 4 ) * 4;
  char* pixels = new char[windowWidth * pitch];

  ScreenshotInfo* info = new ScreenshotInfo { file, windowWidth, windowHeight, pixels };

  glReadPixels( 0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, info->pixels );
  screenshotThread.start( "screenshot", Thread::JOINABLE, screenshotMain, info );
}

void Window::minimise()
{
#if defined( __native_client__ )
#elif SDL_MAJOR_VERSION < 2
  SDL_WM_GrabInput( SDL_GRAB_OFF );
  SDL_WM_IconifyWindow();
#else
  SDL_MinimizeWindow( descriptor );
#endif
}

bool Window::resize( int newWidth, int newHeight, bool fullscreen_ )
{
#ifdef __native_client__

  static_cast<void>( fullscreen_ );

  windowWidth  = newWidth;
  windowHeight = newHeight;

  MainCall() << []()
  {
    glSetCurrentContextPPAPI( 0 );
    context->ResizeBuffers( windowWidth, windowHeight );
    glSetCurrentContextPPAPI( context->pp_resource() );
  };

#elif SDL_MAJOR_VERSION < 2
# ifdef _WIN32

  static_cast<void>( newWidth );
  static_cast<void>( newHeight );
  static_cast<void>( fullscreen_ );

# else

  windowWidth  = windowWidth  == 0 ? screenWidth  : newWidth;
  windowHeight = windowHeight == 0 ? screenHeight : newHeight;
  fullscreen   = fullscreen_;

  uint flags = SDL_OPENGL | ( fullscreen ? SDL_FULLSCREEN : 0 );

  Log::print( "Resizing OpenGL window to %dx%d [%s] ... ",
              windowWidth, windowHeight, fullscreen ? "fullscreen" : "windowed" );

  SDL_FreeSurface( descriptor );
  descriptor = SDL_SetVideoMode( windowWidth, windowHeight, 0, flags );

  if( descriptor == nullptr ) {
    Log::printEnd( "Window resize failed" );
    return false;
  }

  Log::printEnd( "OK" );

# endif
#else

  windowWidth  = windowWidth  == 0 ? screenWidth  : newWidth;
  windowHeight = windowHeight == 0 ? screenHeight : newHeight;
  fullscreen   = fullscreen_;

  Log::print( "Resizing OpenGL window to %dx%d [%s] ... ",
              windowWidth, windowHeight, fullscreen ? "fullscreen" : "windowed" );

  if( fullscreen ) {
    SDL_SetWindowSize( descriptor, windowWidth, windowHeight );
    SDL_SetWindowFullscreen( descriptor, SDL_TRUE );
  }
  else {
    SDL_SetWindowFullscreen( descriptor, SDL_FALSE );
    SDL_SetWindowSize( descriptor, windowWidth, windowHeight );
  }

  Log::printEnd( "OK" );

#endif

  return true;
}

bool Window::create( const char* title, int width, int height, bool fullscreen_ )
{
  measureScreen();

  windowWidth  = width  == 0 ? screenWidth  : width;
  windowHeight = height == 0 ? screenHeight : height;
  fullscreen   = fullscreen_;
  windowFocus  = true;
  windowGrab   = true;

#ifdef __native_client__

  static_cast<void>( title );

  windowWidth  = screenWidth;
  windowHeight = screenHeight;

  flushSemaphore.init();

  Log::print( "Creating OpenGL surface %dx%d ... ", windowWidth, windowHeight );

  MainCall() << []()
  {
    glInitializePPAPI( pp::Module::Get()->get_browser_interface() );

    int attribs[] = {
      PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 24,
      PP_GRAPHICS3DATTRIB_WIDTH, windowWidth,
      PP_GRAPHICS3DATTRIB_HEIGHT, windowHeight,
      PP_GRAPHICS3DATTRIB_NONE
    };

    context = new pp::Graphics3D( System::instance, pp::Graphics3D(), attribs );

    if( context->is_null() ) {
      Log::printEnd( "Failed to create OpenGL context" );
    }
    else if( !System::instance->BindGraphics( *context ) ) {
      Log::printEnd( "Failed to bind Graphics3D" );
    }
    else {
      glSetCurrentContextPPAPI( context->pp_resource() );

      glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
      glFlush();

      context->SwapBuffers( pp::CompletionCallback( flushCompleteCallback, nullptr ) );
    }
  };
  flushSemaphore.wait();

  Log::printEnd( "OK" );

#else

  // Don't mess with screensaver. In X11 it only makes effect for windowed mode, in fullscreen
  // mode screensaver never starts anyway. Turning off screensaver has a side effect: if the game
  // crashes, it remains turned off. Besides that, in X11 several programs (e.g. IM clients) rely
  // on screensaver's counter, so they don't detect that you are away if the screensaver is screwed.
#if SDL_MAJOR_VERSION < 2
  char allowScreensaverEnv[] = "SDL_VIDEO_ALLOW_SCREENSAVER=1";
  SDL_putenv( allowScreensaverEnv );
#else
  SDL_EnableScreenSaver();
#endif

#if SDL_MAJOR_VERSION < 2

  uint flags = SDL_OPENGL | ( fullscreen ? SDL_FULLSCREEN : 0 );

  Log::print( "Creating OpenGL window %dx%d [%s] ... ",
              windowWidth, windowHeight, fullscreen ? "fullscreen" : "windowed" );

  SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE,   0 );
  SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 0 );
  SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 1 );

  descriptor = SDL_SetVideoMode( windowWidth, windowHeight, 0, flags );

  if( descriptor == nullptr ) {
    Log::printEnd( "Window creation failed" );
    return false;
  }

  SDL_WM_SetCaption( title, title );

#else

  uint flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | ( fullscreen ? SDL_WINDOW_FULLSCREEN : 0 );

  Log::print( "Creating OpenGL window %dx%d [%s] ... ",
              windowWidth, windowHeight, fullscreen ? "fullscreen" : "windowed" );

  descriptor = SDL_CreateWindow( title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                 windowWidth, windowHeight, flags );
  if( descriptor == nullptr ) {
    Log::printEnd( "Window creation failed" );
    return false;
  }

# ifdef GL_ES_VERSION_2_0
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK,  SDL_GL_CONTEXT_PROFILE_ES );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
# else
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK,  SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
# endif
  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE,            24 );
  SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE,            0 );
  SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE,          0 );

  SDL_GL_SetSwapInterval( 1 );

  context = SDL_GL_CreateContext( descriptor );

#endif

  Log::printEnd( "OK" );

  glViewport( 0, 0, windowWidth, windowHeight );
  glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
  glFlush();

#if SDL_MAJOR_VERSION < 2
  SDL_GL_SwapBuffers();
#else
  SDL_GL_SwapWindow( descriptor );
#endif
#endif

  return true;
}

void Window::destroy()
{
#if defined( __native_client__ )

  if( !context->is_null() ) {
    MainCall() << []()
    {
      glSetCurrentContextPPAPI( 0 );
      delete context;
      context = nullptr;
      glTerminatePPAPI();
    };
    flushSemaphore.destroy();
  }

#else

  if( descriptor != nullptr ) {
# if SDL_MAJOR_VERSION < 2
    SDL_FreeSurface( descriptor );
# else
    SDL_GL_DeleteContext( context );
    SDL_DestroyWindow( descriptor );
# endif
    descriptor = nullptr;
  }

#endif

  if( screenshotThread.isValid() ) {
    screenshotThread.join();
  }
}

}
