#include <string.h>
#include "workouts.h"

// TODO add uniques counter to bus - AND implement
// TODO add most recent workouts vector to bus - AND implement
//
// TODO make id generation more human friendly --> Assisted Pistol Squat -> apsq
// TODO make id generation avoid collisions --> 

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

    return EXIT_SUCCESS;
}
