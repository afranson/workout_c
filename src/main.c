#include <string.h>
#include "workouts.h"

/* TODO Make sure things work properly for long entries */
/* TODO Refactor 'wid' objects to be smaller and easier to reason */
/* TODO Add ability to rewrite file with new headers/fields */
/* TODO Config file so user can define what fields they want */
/* TODO Make config editable on command line (as long as config options stays small) */
/* TODO Hash table for recent_workouts so it doesn't have to loop over itself each time for membership */
/* TODO Add const to functions that accept pointers where appropriate */
/* TODO Don't allow name modification when editing/progressing workouts */
/* TODO Make name modification when editing or progressing insert an rm of the prior name after its entry */
/* TODO Import data from .csv/other programs? What kind of formats are people using for their workouts? */


int
main(int argc, char **argv)
{
    // Initialize omnibus
    struct bus mainbus = bus_init(argc, argv, "../data/workoutinfo.txt");

    // Parse options
    mainbus.method = bus_parse_argv(&mainbus);

    // Resolve create, help, and broken methods (don't require file reads)
    bus_handle_create_help_broken_methods(&mainbus);

    // Open file for reading
    mainbus.workoutFile = bus_open_workoutfile(&mainbus);

    // Get file length (for memory allocation)
    mainbus.num_workouts = bus_get_num_workouts(&mainbus);

    // Allocate space for workouts
    bus_malloc_workouts(&mainbus);

    // Read file contents into memory
    bus_read_workoutfile(&mainbus);

    // Close file
    bus_close_workoutfile(&mainbus);

    // Perform wid (with id) actions
    workouts_wid_actions(&mainbus);

    // Print final state to user
    workouts_print_workouts(&mainbus);

    // Free mainbus
    free_bus(&mainbus);
    
    return EXIT_SUCCESS;
}
