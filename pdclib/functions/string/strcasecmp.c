//
// Created by artypoole on 18/08/24.
//



#include "ctype.h"
#include "string.h"

/* Compare S1 and S2, ignoring case, returning less than, equal to or
   greater than zero if S1 is lexicographically less than,
   equal to or greater than S2.  */
int strcasecmp(const char* s1, const char*s2)
{
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    int result;

    if (p1 == p2)
        return 0;

    while ((result = tolower(*p1) - tolower(*p2++)) == 0)
        if (*p1++ == '\0')
            break;

    return result;
}

