#ifndef __HTML_UTIL__
#define __HTML_UTIL__

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int stringcompare(const char *s1, const char *s2, size_t length);
char *stringduplicate_length(const char *string, size_t len);
const char *stringtrim_l(const char *string);

#endif