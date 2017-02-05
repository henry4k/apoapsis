#ifndef __KONSTRUKT_FS_UTILS__
#define __KONSTRUKT_FS_UTILS__

#include <vec.h>


static const int MAX_PATH_SIZE = 256;

#if defined(_WIN32)
static const char NATIVE_DIR_SEP = '\\';
#else
static const char NATIVE_DIR_SEP = '/';
#endif

enum FileType
{
    FILE_TYPE_INVALID,
    FILE_TYPE_UNKNOWN,
    FILE_TYPE_REGULAR,
    FILE_TYPE_DIRECTORY
};


struct Path
{
    char str[MAX_PATH_SIZE];
};

typedef vec_t(Path) PathList;


FileType GetFileType( const char* path );

bool CreateDirectory( const char* path );

/**
 * @return
 * Path to the newly created directory or `NULL` if something went wrong.
 */
const char* CreateTemporaryDirectory( const char* name );

/**
 * Lists all files in a directory.  Only real entries are emitted, nothing
 * like `..` or `.`.
 *
 * @return
 * A list of directory entries.
 * Use #FreePathList when you don't need it anymore.
 */
PathList* GetDirEntries( const char* path );

void FreePathList( PathList* list );

/**
 * Deletes files and empty directories.
 */
bool RemoveFile( const char* path );

/**
 * Deletes files and directories recursively.
 *
 * Should be obviously used with caution.
 */
bool RemoveDirectoryTree( const char* path );

#endif
