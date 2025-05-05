// ArtOS - hobby operating system by Artie Poole
// Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>

//
// Created by artiepoole on 4/27/25.
//

#include "art_string.h"
#include "memory.h"

#if SIMD
#include "SIMD.h"
#endif

namespace art_string
{
    void* memset(void* s, const int c, size_t n)
    {
#if SIMD
        return simd_set(s, c, n);
#else

        unsigned char* p = static_cast<unsigned char*>(s);

        while (n--)
        {
            *p++ = static_cast<unsigned char>(c);
        }

        return s;
#endif
    }

    int strncmp(const char* s1, const char* s2, size_t n)
    {
        while (n && *s1 && (*s1 == *s2))
        {
            ++s1;
            ++s2;
            --n;
        }

        if (n == 0)
        {
            return 0;
        }
        else
        {
            return (*(unsigned char*)s1 - *(unsigned char*)s2);
        }
    }

    char* strncpy(char* dest, const char* src, size_t n)
    {
        char* rc = dest;

        while (n && (*dest++ = *src++))
        {
            /* Cannot do "n--" in the conditional as size_t is unsigned and we have
               to check it again for >0 in the next loop below, so we must not risk
               underflow.
            */
            --n;
        }

        /* Checking against 1 as we missed the last --n in the loop above. */
        while (n-- > 1)
        {
            *dest++ = '\0';
        }

        return rc;
    }


    char* strcpy(char* dest, const char* src)
    {
        char* rc = dest;

        while ((*dest++ = *src++))
        {
            /* EMPTY */
        }

        return rc;
    }

    void* memcpy(void* dest, const void* src, size_t n)
    {
#if SIMD
        simd_copy(dest, src, n);
#else
        auto s1 = static_cast<char*>(dest);
        auto s2 = static_cast<const char*>(src);

        while (n--)
        {
            *s1++ = *s2++;
        }
#endif
        return dest;
    }

    size_t strlen(const char* s)
    {
        int len = 0;
        while (s[len])
            len++;
        return len;
    }

    int strcmp(const char* s1, const char* s2)
    {
        while ((*s1) && (*s1 == *s2))
        {
            ++s1;
            ++s2;
        }

        return (*(unsigned char*)s1 - *(unsigned char*)s2);
    }


    char* strdup(const char* s)
    {
        size_t len = strlen(s) + 1;
        void* new_str = art_alloc(len);

        if (new_str == NULL)
            return NULL;

        return static_cast<char*>(memcpy(new_str, s, len));
    }

    char* strndup(const char* s, const size_t n)
    {
        const auto new_str = static_cast<char*>(art_alloc(n + 1));

        if (new_str == NULL)
            return NULL;
        new_str[n] = '\0';
        return static_cast<char*>(memcpy(new_str, s, n));
    }
}
