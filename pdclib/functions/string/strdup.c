//
// Created by artypoole on 18/08/24.
//
# include "stdlib.h"
# include "string.h"

/* Duplicate S, returning an identical malloc'd string.  */
char* strdup(const char* s)
{
    size_t len = strlen(s) + 1;
    void* new = malloc(len);

    if (new == NULL)
        return NULL;

    return (char*)memcpy(new, s, len);
}

char* strndup(const char* s, const size_t n)
{
    char* new = malloc(n + 1);

    if (new == NULL)
        return NULL;
    new[n] = '\0';
    return memcpy(new, s, n);
}
