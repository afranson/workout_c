#include <string.h>
#include "workouts.h"

/* TODO Hash table for ids and avoid collisions */
/* TODO Hash table to recent_workouts so it doesn't have to loop over itself each time for membership */

int
main(int argc, char **argv)
{
    // initialize omnibus
    struct bus mainbus = bus_init(argc, argv, "../data/workoutinfo.txt");

    // parse options
    mainbus.method = bus_parse_argv(&mainbus);

    // resolve create, help, and broken methods (don't require file reads)
    bus_handle_create_help_broken_methods(&mainbus);

    // open file for reading
    mainbus.workoutFile = bus_safe_open_workoutfile(&mainbus);

    // get file length (for memory allocation)
    mainbus.num_workouts = bus_get_num_workouts(&mainbus);

    // read file contents into memory
    bus_read_workoutfile(&mainbus);

    // close file
    bus_safe_close_workoutfile(&mainbus);

    // perform wid (with id) actions
    workouts_wid_actions(&mainbus);

    // print final state to user
    workouts_print_workouts(&mainbus);

    /* Free mainbus completely */
    free(mainbus.filename);
    for (size_t i=0; i<mainbus.num_workouts; i++) {
	free(mainbus.workouts[i].id);
	free(mainbus.workouts[i].exercise); /* Frees all fields due to how split_strings work */
    }
    free(mainbus.workouts);
    free(mainbus.recent_workouts);

    return EXIT_SUCCESS;
}
