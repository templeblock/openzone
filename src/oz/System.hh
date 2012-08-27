/*
 * liboz - OpenZone core library.
 *
 * Copyright © 2002-2012 Davorin Učakar
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
 * @file oz/System.hh
 *
 * System class.
 */

#pragma once

#include "common.hh"

// FIXME Add a space after elipsis in variadic macros once KDevelop gets that bug fixed.

/**
 * @def OZ_WARNING
 * Wrapper for `oz::System::warning()`, filling in the current function, file and line parameters.
 */
#define OZ_WARNING( ...) \
  oz::System::warning( __PRETTY_FUNCTION__, __FILE__, __LINE__, 0, __VA_ARGS__ )

/**
 * @def OZ_ERROR
 * Wrapper for `oz::System::error()`, filling in the current function, file and line parameters.
 */
#define OZ_ERROR( ...) \
  oz::System::error( __PRETTY_FUNCTION__, __FILE__, __LINE__, 0, __VA_ARGS__ )

#ifdef __native_client__

namespace pp
{

// Forward declarations for NaCl classes.
class Module;
class Instance;
class Core;

}

#endif

namespace oz
{

/**
 * Class for generating stack trace for the current function call.
 */
class System
{
  public:

    /// Wait for CTRL-C (or other fatal signal) in `abort()`, so one has time to attach a debugger.
    static const int HALT_BIT = 0x01;

    /// Catch fatal signals (SIGILL, SIGABRT, SIGFPE and SIGSEGV), upon which print diagnostics and
    /// abort the program (similar to `error()` call).
    static const int SIGNAL_HANDLER_BIT = 0x10;

    /// Override handlers for exception violations (`std::terminate()` and
    /// `std::unexpected()`) with handlers that print diagnostics and abort the program via
    /// `error()` call.
    static const int EXCEPTION_HANDLERS_BIT = 0x20;

    /// Handlers bitmask.
    static const int HANDLERS_MASK = 0xf0;

#ifdef __native_client__

    static pp::Module*   module;   ///< NaCl module.
    static pp::Instance* instance; ///< NaCl instance.
    static pp::Core*     core;     ///< NaCl pp::Core interface.

#endif

  private:

    static System system; ///< Private instance, takes care for static (de)initialisation.

    /**
     * Sets up bell and disables `SIGTRAP` handler on Linux, since the default handler would crash
     * the application on `trap()` call.
     */
    System();

    /**
     * Destructor delays normal process termination until the bell finishes playing.
     */
    ~System();

  public:

    /**
     * Abort program.
     *
     * If `HALT_BIT` was passed in on initialisation and `preventHalt` is false, program is halted
     * and waits for a fatal signal before it is aborted, so a debugger can be attached.
     */
    OZ_NORETURN
    static void abort( bool preventHalt = false );

    /**
     * Trigger a breakpoint.
     *
     * It raises `SIGTRAP` on Linux or calls `DebugBreak()` on Windows.
     */
    static void trap();

    /**
     * Play a sound alert.
     *
     * Sine wave from `oz/bellSample.inc` is played asynchronously through PulseAudio on Linux or
     * Win32 API on Windows.
     */
    static void bell();

    /**
     * Print warning message.
     *
     * This function first triggers breakpoint with `trap()`, prints error message, file location
     * and stack trace (skipping `nSkippedFrames` stack frames relative to the caller) to log and
     * plays a bell.
     *
     * You will probably want to use `OZ_WARNING` macro instead to fill in the current function,
     * file and line for you.
     */
    OZ_PRINTF_FORMAT( 5, 6 )
    static void warning( const char* function, const char* file, int line, int nSkippedFrames,
                         const char* msg, ... );

    /**
     * Print error message and halt the program.
     *
     * Same as `System::warning()` but also aborts the program.
     *
     * You will probably want to use `OZ_ERROR` macro instead to fill in the current function, file
     * and line for you.
     */
    OZ_NORETURN
    OZ_PRINTF_FORMAT( 5, 6 )
    static void error( const char* function, const char* file, int line, int nSkippedFrames,
                       const char* msg, ... );

    /**
     * Initialise `System` features.
     *
     * Set-up crash handlers for cases specified in `flags`. If `HALT_BIT` is also given, crash
     * handlers wait for CTRL-C before exit.
     */
#if defined( NDEBUG ) || defined( __native_client__ )
    static void init( int flags = HANDLERS_MASK );
#else
    static void init( int flags = HANDLERS_MASK | HALT_BIT );
#endif

    /**
     * Deinitialise `System`.
     *
     * It resets signal handlers, `std::terminate()` and `std::unexpected()` to defaults.
     */
    static void free();

};

}
