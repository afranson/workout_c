#include <string.h>
#include "workouts.h"

/* TODO Hash table for ids and avoid collisions */
/* TODO Hash table to recent_workouts so it doesn't have to loop over itself each time for membership */

int
main(int argc, char **argv)
{
    // initialize omnibus
    struct bus mainbus = workouts_init_bus(argc, argv, "../data/workoutinfo.txt");

    // parse options
    mainbus.method = workouts_parse_options(&mainbus);

    // resolve create, help, and broken methods (don't require file reads)
    workouts_handle_create_help_broken_methods(&mainbus);

    // open file for reading
    mainbus.workoutFile = workouts_safe_open_workoutfile(&mainbus);

    // get file length (for memory allocation)
    mainbus.num_workouts = workouts_get_num_workouts(&mainbus);

    // read file contents into memory
    workouts_read_workoutfile_into_bus(&mainbus);

    // close file
    workouts_safe_close_workoutfile(&mainbus);

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
