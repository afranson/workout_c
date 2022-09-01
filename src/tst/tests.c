/* Runs unit tests for workouts program */

#include "../workouts.h"
#include <assert.h>

int main()
{
    assert(EXIT_SUCCESS == test_strsplit());

    return EXIT_SUCCESS;
}

int test_strsplit(void)
/* Function to ensure basic string splitting and freeing capabilities */
{
    struct split_string splits = strsplit("This is a string to be split.   ", ' ');
    print_split_string(splits);
    free_split_string(splits);
    return EXIT_SUCCESS;
}
