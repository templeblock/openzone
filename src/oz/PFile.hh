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
 * @file oz/PFile.hh
 *
 * PFile class.
 */

#pragma once

#include "File.hh"

namespace oz
{

/**
 * PhysicsFS wrapper, similar to <tt>File</tt> class.
 *
 * @ingroup oz
 */
class PFile
{
  private:

    String     filePath; ///< %File path.
    File::Type fileType; ///< %File type (initially <tt>MISSING</tt>).
    int        fileSize; ///< %File size (>= 0 if <tt>fileType == REGULAR</tt>, -1 otherwise).
    long64     fileTime; ///< Modification or creation time, what is newer.
    char*      data;     ///< Mapped memory.

  public:

    /**
     * Create an empty instance (path is set to "").
     */
    PFile();

    /**
     * Destructor.
     */
    ~PFile();

    /**
     * Copy constructor.
     *
     * Mapped memory is not copied.
     */
    PFile( const PFile& );

    /**
     * Move constructor, transfers mapped memory.
     */
    PFile( PFile&& file );

    /**
     * Copy operator.
     *
     * Mapped memory is not copied.
     */
    PFile& operator = ( const PFile& );

    /**
     * Move operator, transfers mapped memory.
     */
    PFile& operator = ( PFile&& file );

    /**
     * Create an instance for the given path.
     */
    explicit PFile( const char* path );

    /**
     * Set a new file path.
     */
    void setPath( const char* path );

    /**
     * Access file to get its type and size.
     *
     * @return true iff stat succeeds, i.e. file exists.
     */
    bool stat();

    /**
     * %File type.
     *
     * <tt>stat()</tt> function must be called first to fill type and size properties. Initial
     * values are <tt>MISSING</tt> and -1 respectively.
     */
    File::Type type() const;

    /**
     * Modification or creation (Unix) time, what is newer.
     *
     * %File must be stat'd before.
     */
    long64 time() const;

    /**
     * %File size in bytes if regular file, -1 otherwise.
     *
     * <tt>stat()</tt> function must be called first to fill type and size properties. Initial
     * values are <tt>MISSING</tt> and -1 respectively.
     */
    int size() const;

    /**
     * %File path.
     */
    const String& path() const;

    /**
     * %Path in real filesystem to file's archive or top folder that is mouted to VFS.
     */
    String realDir() const;

    /**
     * Mount point under which file's archive or top directory is mounted.
     */
    String mountPoint() const;

    /**
     * %File name.
     */
    String name() const;

    /**
     * Extension (part of base name after the last dot) or "" if no dot in base name.
     */
    String extension() const;

    /**
     * Name without the extension (and the dot).
     */
    String baseName() const;

    /**
     * True iff the extension (without dot) is equal to the given string.
     *
     * Empty string matches both no extension and files ending with dot.
     */
    bool hasExtension( const char* ext ) const;

    /**
     * True iff file is mapped to memory.
     */
    bool isMapped() const;

    /**
     * %Map file into memory for reading.
     *
     * It also sets file type on <tt>REGULAR</tt> and updates file size if map succeeds.
     */
    bool map();

    /**
     * Unmap mapped file.
     */
    void unmap();

    /**
     * Get <tt>InputStream</tt> for currently mapped file.
     */
    InputStream inputStream( Endian::Order order = Endian::NATIVE ) const;

    /**
     * Read file into a buffer.
     */
    Buffer read();

    /**
     * Write buffer contents to the file.
     *
     * It also sets file type on <tt>REGULAR</tt> and updates file size if it succeeds.
     * Write operation is not possible while file is mapped.
     */
    bool write( const char* buffer, int size );

    /**
     * Write buffer contents into a file.
     *
     * It also sets file type on <tt>REGULAR</tt> and updates file size if it succeeds.
     * Write operation is not possible while file is mapped.
     */
    bool write( const Buffer* buffer );

    /**
     * Generate a list of files in directory.
     *
     * Hidden files (in Unix means, so everything starting with '.') are skipped.
     * On error, empty array is returned.
     *
     * Directory listing is not supported on NaCl, so this function always returns an empty list.
     */
    DArray<PFile> ls();

    /**
     * Make a new directory.
     *
     * This function always fails on NaCl since directories are not supported.
     */
    static bool mkdir( const char* path );

    /**
     * Delete a file or an empty directory.
     *
     * This function always fails on NaCl since file deletion is not supported.
     */
    static bool rm( const char* path );

    /**
     * Mount read-only directoy or archive to VFS.
     *
     * @param path archive or directory in real file system directory to mount.
     * @param mountPoint mount point in VFS, "" or <tt>null</tt> equals root of VFS.
     * @param append true to add to the end instead to the beginning of the search path.
     */
    static bool mount( const char* path, const char* mountPoint, bool append );

    /**
     * Mount read/write local resource directory to root of VFS.
     *
     * This function does not fork for NaCl.
     *
     * @param path path to directory in real file system.
     */
    static bool mountLocal( const char* path );

    /**
     * Initialise PhysicsFS.
     *
     * On NaCl <tt>System::instance</tt> must be set prior to initialisation of PhysicsFS.
     */
    static void init( File::FilesystemType type = File::TEMPORARY, int size = 0 );

    /**
     * Deinitialise PhysicsFS.
     */
    static void free();

};

}