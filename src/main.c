#include <string.h>
#include "workouts.h"

/* TODO Hash table for ids and avoid collisions */
/* TODO Hash table to recent_workouts so it doesn't have to loop over itself each time for membership */
/* TODO make progress with an old rm'd workout bring it back to life */

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

    // Free mainbus completely
    free_bus(&mainbus);
    
    return EXIT_SUCCESS;
}
