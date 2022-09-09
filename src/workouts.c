#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "workouts.h"


/* Default, empty objects for initializing */
struct workout workout_default = {.active=false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};


/* Returns the index of the most recent (latest in list) workout with id matching
   the given exercise */
struct size_t_w_error
workouts_get_most_recent_workout(struct bus *mainbus, char *id, size_t max_i)
{
    for (int i=max_i; i>=0; i--) {
        if( !strcmp(mainbus->workouts[i].id, id) ) {
	    return (struct size_t_w_error){.value=i, .error=false};
        }
    }
    fprintf(stderr, "Error: Failed to find instance of workout id, %s.\n", id);
    return (struct size_t_w_error){.value=0, .error=true};
}


/* The final printing step for each of the cases */
void
workouts_print_workouts(struct bus *mainbus)
{
    /* TODO Figure out/implement how I want to sort the workouts (by date?) */

    qsort(mainbus->recent_workouts, mainbus->num_uniques, sizeof(*mainbus->recent_workouts), compare_date);
    
    switch ( mainbus->method ) {
    case list_wid:
	break;
    default:
	workout_pprint_header();
    }
    for (size_t i=0; i<mainbus->num_uniques; i++) {
	struct workout workout = mainbus->recent_workouts[i];
        switch ( mainbus->method ) {
        case all:
	    workout_pprint(workout);
            break;
        case show:
	case progress_wid:
        case edit_wid:
	case rm_wid:
            if ( workout.active ) {
		workout_pprint(workout);
            }
            break;
        default:
            break;
        }
    }
    return;
}


/* Comparison algorithm for sorting  */
int
compare_date(const void *a, const void *b)
{
    const struct workout *workout_1 = (const struct workout *)a;
    const struct workout *workout_2 = (const struct workout *)b;
    long int date_1, date_2;
    char *tmp_ptr;
    date_1 = strtol(workout_1->date, &tmp_ptr, 10);
    date_2 = strtol(workout_2->date, &tmp_ptr, 10);
    return date_1 - date_2;
}


void
workouts_wid_actions(struct bus *mainbus)
{
    switch( mainbus->method ) {
        case progress_wid:
            workouts_progress_wid_workout(mainbus, mainbus->argv[2]);
            break;
        case rm_wid:
            workouts_rm_wid_workout(mainbus, mainbus->argv[2]);
            break;
        case list_wid:
            workouts_list_wid_workout(mainbus, mainbus->argv[2]);
            break;
        case edit_wid:
            workouts_edit_wid_workout(mainbus, mainbus->argv[2]);
            break;
        default:
            break;
    }
    return;
}


int
workouts_progress_wid_workout(struct bus *mainbus, char *id)
{
    // open file for appending
    mainbus->workoutFile = bus_open_workoutfile_append(mainbus);

    struct size_t_w_error most_current_index = workouts_get_most_recent_workout(mainbus, id, mainbus->num_workouts-1);
    if ( most_current_index.error ) {
	printf("\nWorkout id '%s' is not valid. Please see valid id's seen on leftmost column after using the 'all' command below.\n\n", id);
	mainbus->method = all;
	return EXIT_FAILURE;
    }
    
    struct workout temp_workout = mainbus->workouts[most_current_index.value];

    // fill fields with proper defaults
    struct split_string default_workout_ss = workout_to_split_string(temp_workout);
    char **default_workout = default_workout_ss.str_p_array;
    char *todays_date = get_todays_date_yymmdd();
    
    default_workout[5] = todays_date;

    temp_workout = workouts_generate_workout(default_workout);

    // Then write full
    bus_write_workout(mainbus, temp_workout);
    mainbus->workouts[mainbus->num_workouts] = temp_workout;
    bus_update_recent_workouts(mainbus, mainbus->workouts[mainbus->num_workouts]);

    free_split_string(default_workout_ss);
    free(todays_date);

    // close file
    bus_close_workoutfile(mainbus);
    return EXIT_SUCCESS;
}


int
workouts_rm_wid_workout(struct bus *mainbus, char *id)
{
    // open file for appending
    mainbus->workoutFile = bus_open_workoutfile_append(mainbus);

    // get matching workout
    struct size_t_w_error most_current_index = workouts_get_most_recent_workout(mainbus, id, mainbus->num_workouts-1);
    if ( most_current_index.error ) {
	printf("\nWorkout id '%s' is not valid. Please see valid id's seen on leftmost column after using the 'show' command below.\n\n", id);

	mainbus->method = show;
	return EXIT_FAILURE;
    }
    
    struct workout temp_workout = mainbus->workouts[most_current_index.value];

    // Then write rm field
    workouts_write_rm_workout(mainbus->workoutFile, temp_workout);
    // Update mainbus
    mainbus->workouts[mainbus->num_workouts] = create_rm_workout(temp_workout.exercise);
    bus_update_recent_workouts(mainbus, mainbus->workouts[mainbus->num_workouts]);

    // close file
    bus_close_workoutfile(mainbus);
    return EXIT_SUCCESS;
}


int
workouts_write_rm_workout(FILE* workoutFile, struct workout workout_to_add)
{
    return fprintf(workoutFile, "%s|%s\n", workout_to_add.exercise, "rm");
}


int
workouts_list_wid_workout(struct bus *mainbus, char *id)
{
    // print all matching workouts
    workout_pprint_header();
    for (size_t i=0; i < mainbus->num_workouts; i++) {
        if( !strcmp(mainbus->workouts[i].id, id)
	    && strcmp(mainbus->workouts[i].notes, "rm") ) { /* ID matches and notes is not "rm" */
	    workout_pprint(mainbus->workouts[i]);
        }
    }
    return EXIT_SUCCESS;
}


void
workouts_edit_wid_workout(struct bus *mainbus, char *id)
{
    // open temp file for writing and main file for reading
    struct bus tempbus = bus_default;
    tempbus.filename = "tmp_workoutseditfile.txt";
    tempbus.workoutFile = bus_open_workoutfile_append(&tempbus);
    mainbus->workoutFile = bus_open_workoutfile(mainbus);

    struct size_t_w_error most_current_index = workouts_get_most_recent_workout(mainbus, id, mainbus->num_workouts-1);
    if ( most_current_index.error ) {\
	printf("\nWorkout id '%s' is not valid. Please see valid id's seen on leftmost column after using the 'show' command below.\n\n", id);
	mainbus->method = show;
	return;
    }

    struct workout previous_workout = mainbus->workouts[most_current_index.value];
    struct workout generated_workout;

    // fill fields with proper defaults
    struct split_string default_workout_ss = workout_to_split_string(previous_workout);
    char **default_workout = default_workout_ss.str_p_array;
    generated_workout = workouts_generate_workout(default_workout);

    // Then write whole file replacing just the edited workout
    workouts_write_edited_workout(mainbus, &tempbus, previous_workout, generated_workout);

    free_split_string(default_workout_ss);

    // close files
    bus_close_workoutfile(mainbus);
    bus_close_workoutfile(&tempbus);

    // move temp file to overwrite real one
    rename(tempbus.filename, mainbus->filename);

    return;
}


int
workouts_write_edited_workout(struct bus *mainbus, struct bus *tempbus, struct workout original_workout, struct workout edited_workout)
{
    char *workout_line;
    struct workout woi;
    for ( size_t i=0; i<mainbus->num_workouts; ++i ) {
	woi = mainbus->workouts[i];
	if ( workout_compare(woi, original_workout) ) /* If they're the same */
        {
	    /* Replace the workout in file and in bus */
            bus_write_workout(tempbus, edited_workout);
	    mainbus->workouts[i] = edited_workout;
	    bus_update_recent_workouts(mainbus, mainbus->workouts[i]);
        } else {
            // write previous
	    workout_line = workout_to_string(woi);
	    fputs(workout_line, tempbus->workoutFile);
	    free(workout_line);
        }
    }
    return EXIT_SUCCESS;
}


struct workout
workouts_generate_workout(char **default_options)
{
    /* TODO maybe ncurses to prefill stdin with default values */
    /* Create a new workout and makes it's fields point at the stdin input */
    struct workout generated_workout = workout_default;
    char *workout_pointers[7];
    char *buffer = malloc(MAX_WORKOUT_SIZE);

    // collect fields from user
    const char *fields[] = {"Name", "Weights", "Sets", "Reps", "Days", "Date", "Notes"};
    printf("Enter Name then Weights ... Sets Reps Days Date Notes\n");
    if ( default_options == NULL ) {
        for (int i=0; i<7; i++)
        {
            printf("%s: ", fields[i]);
	    fgets(buffer, MAX_WORKOUT_SIZE, stdin);
	    buffer[strlen(buffer)-1] = '\0'; /* Remove \n at end of input */
	    workout_pointers[i] = strdup(buffer);
        }
    } else {
        for (int i=0; i<7; i++)
        {
            printf("%s [%s]: ", fields[i], default_options[i]);
            fgets(buffer, MAX_WORKOUT_SIZE, stdin);
            if ( buffer[0] == '\n' || !buffer ) {
		workout_pointers[i] = strdup(default_options[i]);
            } else {
		buffer[strlen(buffer)-1] = '\0'; /* Remove \n at end of input */
		workout_pointers[i] = strdup(buffer);
	    }
        }
    }
    free(buffer);

    generated_workout.active = true;
    generated_workout.id = workout_get_id(workout_pointers[0]);
    generated_workout.exercise = workout_pointers[0];
    generated_workout.weights = workout_pointers[1];
    generated_workout.sets = workout_pointers[2];
    generated_workout.reps = workout_pointers[3];
    generated_workout.days = workout_pointers[4];
    generated_workout.date = workout_pointers[5];
    generated_workout.notes = workout_pointers[6]; 
    
    // uncomment to see the whole allocated buffer in hex
    // for (int i=0; i<525; ++i) { printf("%#.2x ", *(*total_buffer + i)); if ((i+1)%50 == 0) printf("\n"); } printf("\n");
    return generated_workout;
}
