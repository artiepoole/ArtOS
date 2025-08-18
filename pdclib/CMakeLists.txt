cmake_minimum_required(VERSION 3.1.3...3.27.7 FATAL_ERROR)
option(SIMD "Replace memcpy with SIMD version?" ON)
project(pdclib_proj LANGUAGES C CXX)

set(TARGET_NAME platform_functions)

set(PDCLIB_SOURCES
        functions/ctype/isalnum.c
        functions/ctype/isalpha.c
        functions/ctype/isblank.c
        functions/ctype/iscntrl.c
        functions/ctype/isdigit.c
        functions/ctype/isgraph.c
        functions/ctype/islower.c
        functions/ctype/isprint.c
        functions/ctype/ispunct.c
        functions/ctype/isspace.c
        functions/ctype/isupper.c
        functions/ctype/isxdigit.c
        functions/ctype/tolower.c
        functions/ctype/toupper.c
        #
        functions/inttypes/imaxabs.c
        functions/inttypes/imaxdiv.c
        functions/inttypes/strtoimax.c
        functions/inttypes/strtoumax.c
        #
        functions/locale/localeconv.c
        functions/locale/setlocale.c
        #
        platform/ArtOS/functions/signal/raise.c
        platform/ArtOS/functions/signal/signal.c

        functions/stdio/clearerr.c
        functions/stdio/fclose.c
        functions/stdio/feof.c
        functions/stdio/ferror.c
        functions/stdio/fflush.c
        functions/stdio/fgetc.c
        functions/stdio/fgetpos.c
        functions/stdio/fgets.c
        functions/stdio/fopen.c
        functions/stdio/fopen_s.c
        functions/stdio/fprintf.c
        functions/stdio/fputc.c
        functions/stdio/fputs.c
        functions/stdio/fread.c
        functions/stdio/freopen.c
        functions/stdio/freopen_s.c
        functions/stdio/fscanf.c
        functions/stdio/fseek.c
        functions/stdio/fsetpos.c
        functions/stdio/ftell.c
        functions/stdio/fwrite.c
        functions/stdio/getc.c
        functions/stdio/getchar.c
        functions/stdio/perror.c
        functions/stdio/printf.c
        functions/stdio/putc.c
        functions/stdio/putchar.c
        functions/stdio/puts.c
        functions/stdio/remove.c
        functions/stdio/rename.c
        functions/stdio/rewind.c
        functions/stdio/scanf.c
        functions/stdio/setbuf.c
        functions/stdio/setvbuf.c
        functions/stdio/snprintf.c
        functions/stdio/sprintf.c
        functions/stdio/sscanf.c
        functions/stdio/tmpfile_s.c
        functions/stdio/tmpnam.c
        functions/stdio/ungetc.c
        functions/stdio/vfprintf.c
        functions/stdio/vfscanf.c
        functions/stdio/vprintf.c
        functions/stdio/vscanf.c
        functions/stdio/vsnprintf.c
        functions/stdio/vsprintf.c
        functions/stdio/vsscanf.c
        #        platform/ArtOS/functions/stdio/tmpfile.cpp

        functions/stdlib/abort.c
        functions/stdlib/abort_handler_s.c
        functions/stdlib/abs.c
        functions/stdlib/at_quick_exit.c
        functions/stdlib/atexit.c
        functions/stdlib/atoi.c
        functions/stdlib/atof.c
        functions/stdlib/atol.c
        functions/stdlib/atoll.c
        functions/stdlib/bsearch.c
        functions/stdlib/bsearch_s.c
        functions/stdlib/div.c
        functions/stdlib/exit.c
        functions/stdlib/_Exit.c
        functions/stdlib/ignore_handler_s.c
        functions/stdlib/labs.c
        functions/stdlib/ldiv.c
        functions/stdlib/llabs.c
        functions/stdlib/lldiv.c
        functions/stdlib/qsort.c
        functions/stdlib/qsort_s.c
        functions/stdlib/quick_exit.c
        functions/stdlib/rand.c
        functions/stdlib/set_constraint_handler_s.c
        functions/stdlib/srand.c
        functions/stdlib/strtol.c
        functions/stdlib/strtoll.c
        functions/stdlib/strtoul.c
        functions/stdlib/strtoull.c
        platform/ArtOS/functions/stdlib/getenv.c
        platform/ArtOS/functions/stdlib/getenv_s.c
        platform/ArtOS/functions/stdlib/system.c
        #
        functions/string/memchr.c
        functions/string/memcmp.c
        functions/string/memcpy.c
        functions/string/memmove.c
        functions/string/memset.c
        functions/string/strcat.c
        functions/string/strchr.c
        functions/string/strcmp.c
        functions/string/strcoll.c
        functions/string/strcpy.c
        functions/string/strcspn.c
        functions/string/strerror.c
        functions/string/strlen.c
        functions/string/strncat.c
        functions/string/strncmp.c
        functions/string/strncpy.c
        functions/string/strpbrk.c
        functions/string/strrchr.c
        functions/string/strspn.c
        functions/string/strstr.c
        functions/string/strtok.c
        functions/string/strtok_s.c
        functions/string/strxfrm.c
        functions/string/memcpy_s.c
        functions/string/memmove_s.c
        functions/string/memset_s.c
        functions/string/strcat_s.c
        functions/string/strcpy_s.c
        functions/string/strerror_s.c
        functions/string/strerrorlen_s.c
        functions/string/strncat_s.c
        functions/string/strncpy_s.c
        functions/string/strcasecmp.c
        functions/string/strdup.c
        functions/string/strncasecmp.c
        #
        functions/time/asctime.c
        functions/time/asctime_s.c
        functions/time/ctime.c
        functions/time/ctime_s.c
        functions/time/difftime.c
        functions/time/gmtime.c
        functions/time/gmtime_s.c
        functions/time/localtime.c
        functions/time/localtime_s.c
        functions/time/mktime.c
        functions/time/strftime.c
        platform/ArtOS/functions/time/clock.cpp
        platform/ArtOS/functions/time/time.cpp
        #        platform/ArtOS/functions/time/timespec_get.c

        functions/_dlmalloc/malloc.c

        functions/_dtoa/dtoa.c

        functions/_tzcode/_PDCLIB_gmtcheck.c
        functions/_tzcode/_PDCLIB_gmtsub.c
        functions/_tzcode/_PDCLIB_increment_overflow.c
        functions/_tzcode/_PDCLIB_init_ttinfo.c
        functions/_tzcode/_PDCLIB_localsub.c
        functions/_tzcode/_PDCLIB_localtime_tzset.c
        functions/_tzcode/_PDCLIB_mktime_tzname.c
        functions/_tzcode/_PDCLIB_timesub.c
        functions/_tzcode/_PDCLIB_tzload.c
        functions/_tzcode/_PDCLIB_tzparse.c
        functions/_tzcode/_PDCLIB_tzset_unlocked.c
        functions/_tzcode/_PDCLIB_update_tzname_etc.c
        #
        functions/_PDCLIB/assert.c
        functions/_PDCLIB/errno.c
        functions/_PDCLIB/_PDCLIB_atomax.c
        functions/_PDCLIB/_PDCLIB_bigint.c
        functions/_PDCLIB/_PDCLIB_bigint2.c
        functions/_PDCLIB/_PDCLIB_bigint10.c
        functions/_PDCLIB/_PDCLIB_bigint32.c
        functions/_PDCLIB/_PDCLIB_bigint64.c
        functions/_PDCLIB/_PDCLIB_bigint_add.c
        functions/_PDCLIB/_PDCLIB_bigint_sub.c
        functions/_PDCLIB/_PDCLIB_bigint_cmp.c
        functions/_PDCLIB/_PDCLIB_bigint_invlog2.c
        functions/_PDCLIB/_PDCLIB_bigint_log2.c
        functions/_PDCLIB/_PDCLIB_bigint_mant.c
        functions/_PDCLIB/_PDCLIB_bigint_mul.c
        functions/_PDCLIB/_PDCLIB_bigint_mul_dig.c
        functions/_PDCLIB/_PDCLIB_bigint_div.c
        functions/_PDCLIB/_PDCLIB_bigint_div_dig.c
        functions/_PDCLIB/_PDCLIB_bigint_shl.c
        functions/_PDCLIB/_PDCLIB_bigint_tostring.c
        functions/_PDCLIB/_PDCLIB_closeall.c
        functions/_PDCLIB/_PDCLIB_digits.c
        functions/_PDCLIB/_PDCLIB_filemode.c
        functions/_PDCLIB/_PDCLIB_getstream.c
        functions/_PDCLIB/_PDCLIB_init_file_t.c
        functions/_PDCLIB/_PDCLIB_is_leap.c
        functions/_PDCLIB/_PDCLIB_isstream.c
        functions/_PDCLIB/_PDCLIB_load_lc_collate.c
        functions/_PDCLIB/_PDCLIB_load_lc_ctype.c
        functions/_PDCLIB/_PDCLIB_load_lc_messages.c
        functions/_PDCLIB/_PDCLIB_load_lc_monetary.c
        functions/_PDCLIB/_PDCLIB_load_lc_numeric.c
        functions/_PDCLIB/_PDCLIB_load_lc_time.c
        functions/_PDCLIB/_PDCLIB_load_lines.c
        functions/_PDCLIB/_PDCLIB_prepread.c
        functions/_PDCLIB/_PDCLIB_prepwrite.c
        functions/_PDCLIB/_PDCLIB_print.c
        functions/_PDCLIB/_PDCLIB_print_fp.c
        functions/_PDCLIB/_PDCLIB_print_integer.c
        functions/_PDCLIB/_PDCLIB_print_string.c
        functions/_PDCLIB/_PDCLIB_scan.c
        functions/_PDCLIB/_PDCLIB_seed.c
        functions/_PDCLIB/_PDCLIB_strtod_main.c
        functions/_PDCLIB/_PDCLIB_strtod_scan.c
        functions/_PDCLIB/_PDCLIB_strtok.c
        functions/_PDCLIB/_PDCLIB_strtox_main.c
        functions/_PDCLIB/_PDCLIB_strtox_prelim.c
        functions/_PDCLIB/stdarg.c

        platform/ArtOS/functions/_PDCLIB/_PDCLIB_changemode.c
        platform/ArtOS/functions/_PDCLIB/_PDCLIB_close.c
        platform/ArtOS/functions/_PDCLIB/_PDCLIB_Exit.cpp
        platform/ArtOS/functions/_PDCLIB/_PDCLIB_fillbuffer.c
        platform/ArtOS/functions/_PDCLIB/_PDCLIB_flushbuffer.cpp
        platform/ArtOS/functions/_PDCLIB/_PDCLIB_open.cpp
        platform/ArtOS/functions/_PDCLIB/_PDCLIB_realpath.c
        #        platform/ArtOS/functions/_PDCLIB/_PDCLIB_rename.c
        platform/ArtOS/functions/_PDCLIB/_PDCLIB_remove.c
        platform/ArtOS/functions/_PDCLIB/_PDCLIB_seek.c
        platform/ArtOS/functions/_PDCLIB/_PDCLIB_stdinit.c
)


file(GLOB DIRS
        LIST_DIRECTORIES true
        include/*
        platform/*)

set(PDCLIB_HEADERS
        include/assert.h
        include/ctype.h
        include/errno.h
        include/inttypes.h
        include/iso646.h
        include/limits.h
        include/locale.h
        include/stdalign.h
        include/stdarg.h
        include/stdbool.h
        include/stddef.h
        include/stdint.h
        include/stdio.h
        include/stdlib.h
        include/stdnoreturn.h
        include/string.h
        include/time.h
        include/wctype.h
        platform/ArtOS/include/float.h
        platform/ArtOS/include/signal.h
)


set(PDCLIB_PRIVATE_HEADERS
        include/pdclib/_PDCLIB_glue.h
        include/pdclib/_PDCLIB_internal.h
        include/pdclib/_PDCLIB_lib_ext1.h
        platform/ArtOS/include/pdclib/_PDCLIB_config.h
        functions/stdlib/atof.c
)

#add_library( pdclib  SHARED ${PDCLIB_SOURCES} ${PDCLIB_HEADERS} ${PDCLIB_PRIVATE_HEADERS} ${PDCLIB_NOINSTALL_HEADERS} ${KERNEL})
add_library(pdclib STATIC ${PDCLIB_SOURCES} ${PDCLIB_HEADERS} ${PDCLIB_PRIVATE_HEADERS} ${PDCLIB_NOINSTALL_HEADERS}) #${KERNEL}

#target_include_directories( pdclib  BEFORE PRIVATE ${CMAKE_BINARY_DIR}/configured_include include platform/ArtOS/include ../src ../src/sys ../include)
target_include_directories(pdclib BEFORE PRIVATE ${CMAKE_BINARY_DIR}/configured_include)
target_include_directories(pdclib PUBLIC
        include
        include/pdclib


        platform/ArtOS/include

        platform/ArtOS/functions

        platform/ArtOS/include

        platform/ArtOS/include/pdclib

        platform/ArtOS/functions/_PDCLIB/
        platform/ArtOS/functions/signal
        platform/ArtOS/functions/stdio
        platform/ArtOS/functions/stdlib
        platform/ArtOS/functions/threads
        platform/ArtOS/functions/time

        functions/_dlmalloc/
        functions/_dtoa
        functions/_PDCLIB
        functions/_tzcode
        functions/ctype
        functions/inttypes
        functions/locale
        functions/stdio
        functions/stdlib
        functions/string
        functions/time

        ../Generic/sys
        ../Generic/sys/Constants
        ../Generic/sys/FileSystem
        ../Generic/sys/ComponentTypes
        ../Generic/sys/Logging
        ../Generic/sys/Memory
        ../Generic/sys/Storage
        ../Generic/sys/Terminal
        ../Specific/
        ../Specific/x86
        ../Specific/x86/CPU
        ../Specific/x86/Timers
        ../Specific/multiboot2


)


set_property(TARGET pdclib PROPERTY POSITION_INDEPENDENT_CODE 1)

set(pdclib CMAKE_C_FLAGS "-Wall -Wextra -pedantic -Wno-unused-parameter -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wmissing-declarations -Wredundant-decls -Winline -Wno-long-long -Wuninitialized -fno-builtin -fvisibility=hidden -D_STDC_PREDEF_H -nostdlib ")

set_source_files_properties(functions/_dtoa/dtoa.c PROPERTIES COMPILE_OPTIONS -Wno-sign-compare)

set_property(TARGET pdclib APPEND_STRING PROPERTY COMPILE_FLAGS "-D_PDCLIB_STATIC_DEFINE ${FLAGS}")

target_compile_definitions(pdclib PUBLIC
        SIMD=$<BOOL:${SIMD}>
)

target_link_libraries(pdclib ArtOSTypes ArtOS_lib)
