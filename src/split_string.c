/* Here is where string manipulating functions are implemented */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "workouts.h"

// Works great, but you don't know the number of splits unless you save it somewhere
struct split_string
strsplit(char* string, char delim)
/* Returns an array of pointers that points to a sacrificial string.
 Freeing the split string requires freeing each pointer within the pointer array and the array itself. */
{
    size_t string_length = strlen(string);
    char* sacrificial_string = malloc(string_length + 1); /* The +1 is VERY important to hold the NULL character */
    strncpy(sacrificial_string, string, string_length);
    char** split_string_array = malloc(sizeof(*split_string_array) * string_length);
    split_string_array[0] = (char *)(sacrificial_string);
    size_t i=0, j=1; /* i traverses the string, j the return pointer array */
    
    /* Modifies sacrificial_string by replacing delimiter locations by \0.
     Then assigns pointers to the location after \0 to start the next string. */
    for ( i=0; i<string_length; i++ ) {
	if ( sacrificial_string[i] == delim ) {
	    sacrificial_string[i] = '\0';
	    split_string_array[j++] = (char *)(sacrificial_string + i + 1);
	}
    }
    sacrificial_string[i] = '\0'; /* Add that all important NULL character */
    
    /* Makes the resulting array of pointers the correct size. */
    char** temp_ss = realloc(split_string_array, j * sizeof(*temp_ss));
    if ( !temp_ss ) {
        perror("Error: Failed to realloc split string!");
    } else {
	split_string_array = temp_ss;
    }
    
    /* returns the proper struct */
    struct split_string return_split_string = { .str_p_array = split_string_array, .num_elements = j};
    return return_split_string;
}


/* Prints all the contents of the string */
void print_split_string(struct split_string ss)
{
    for (size_t i=0; i<ss.num_elements; i++) {
        printf("%s\n", ss.str_p_array[i]);
    }
    return;
}


void free_split_string(struct split_string ss)
/* Frees memory allocated for the struct split_string */
{
    /* Only need to free the first element because that contains the char* for the whole original string */
    free(ss.str_p_array[0]);
    free(ss.str_p_array);
    return;
}
