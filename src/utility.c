/* Here is where general utilities lie
   Such as printing debug information */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "workouts.h"


/* Prints len number of contents from a buffer in hex format
   so that you can 'see' \n and \0 and other special chars */
void
print_buffer(char* buff, size_t len)
{
    for( size_t i=0; i<len; i++ )
    {
	printf("%x ", buff[i]);
    }
    printf("\n");
    return;
}


/* Safe way to remove newline from end of input (from stdin for example) */
void
remove_end_newline(char **bufferp)
{
    size_t len = strlen(*bufferp);
    if (len > 0 && *bufferp[len-1] == '\n') {
	*bufferp[--len] = '\0';
    }
    return;
}
