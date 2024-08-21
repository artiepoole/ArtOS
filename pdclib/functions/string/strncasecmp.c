//
// Created by artypoole on 18/08/24.
//

#include "ctype.h"
#include "string.h"

/* Compare no more than N characters of S1 and S2,
   ignoring case, returning less than, equal to or
   greater than zero if S1 is lexicographically less
   than, equal to or greater than S2.  */
int strncasecmp(const char* s1, const char* s2, size_t n)
{
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    int result;

    if (p1 == p2 || n == 0)
        return 0;

    while ((result = tolower(*p1) - tolower(*p2++)) == 0)
        if (*p1++ == '\0' || --n == 0)
            break;

    return result;
}
