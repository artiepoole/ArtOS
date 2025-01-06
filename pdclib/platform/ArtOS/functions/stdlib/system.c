/* system( const char * )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#include "stdlib.h"

/* This is an ArtOS implementation of system() fit for use with POSIX kernels.
*/

#ifdef __cplusplus
extern "C" {
#endif

int fork(void) { return 0; }

int execve(const char* filename, char* const argv[], char* const envp[])
{
    // TODO: Read an executable and load into memory and do context switching etc.
    return 0;
}

int wait(int* status) { return 0; }
int _PDCLIB_rename(const char* oldpath, const char* newpath) { return 0; }
#ifdef __cplusplus
}
#endif

int system(const char* string)
{
    const char* argv[] = {"sh", "-c", NULL, NULL};
    argv[2] = string;

    if (string != NULL)
    {
        int pid = fork();

        if (pid == 0)
        {
            execve("/bin/sh", (char* const *)argv, NULL);
        }
        else if (pid > 0)
        {
            while (wait(NULL) != pid)
            {
                /* EMPTY */
            }
        }
    }

    return -1;
}

#ifdef TEST

#include "_PDCLIB_test.h"

#define SHELLCOMMAND "echo 'SUCCESS testing system()'"

int main( void )
{
    FILE * fh;
    char buffer[25];
    buffer[24] = 'x';
    TESTCASE( ( fh = freopen( testfile, "wb+", stdout ) ) != NULL );
    TESTCASE( system( SHELLCOMMAND ) );
    rewind( fh );
    TESTCASE( fread( buffer, 1, 24, fh ) == 24 );
    TESTCASE( memcmp( buffer, "SUCCESS testing system()", 24 ) == 0 );
    TESTCASE( buffer[24] == 'x' );
    TESTCASE( fclose( fh ) == 0 );
    TESTCASE( remove( testfile ) == 0 );
    return TEST_RESULTS;
}

#endif
