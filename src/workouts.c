#include <stdio.h>
#include <stdlib.h>
#include <libgen.h> 		/* basename, dirname */
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include "workouts.h"


/* Default, empty objects for initializing */
struct workout workout_default = {.active=false, .next_workout=NULL, .previous_workout=NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
struct bus bus_default = {0, NULL, broken, NULL, NULL, 0, NULL, NULL, 0};


/* Main object that gest passed around during the execution of the program.
   Stores CLI args, parsed CLI args, the filename, number of workouts, the workouts themselves, recent workouts, and number of unique workouts.*/
struct bus
workouts_init_bus(int argc, char **argv, char *filename)
{
    struct bus mainbus = bus_default;
    mainbus.argc = argc;
    mainbus.argv = argv;
    /* mainbus.method = broken; */ 

    char filepath[FILENAME_MAX];
    snprintf(filepath, FILENAME_MAX, "%s/%s", dirname(argv[0]), filename);
    mainbus.filename = strdup(filepath);
    return mainbus;
}


struct string_method_pair
{
    char* string;
    enum methods method;
};

const struct string_method_pair two_args_checks[] = { { "create", create }, { "help", help }, { "all", all }, { "show", show } };
const struct string_method_pair three_args_checks[] = { { "list", list_wid }, { "progress", progress_wid }, { "edit", edit_wid }, { "rm", rm_wid } };
/* Parses the command line args for the program
   If 1 (no args) - default to show
   If 2 - Check for create, help, all, and show
   If 3 - Check for list, progress, edit, and rm. */
enum methods
workouts_parse_options(struct bus *mainbus)
{
    enum methods method = mainbus->method;
    int argc = mainbus->argc;
    char **argv = mainbus->argv;
    if ( argc == 1 ) { 		/* No arguments given */
        method = show;
    } else if ( argc == 2 ) { 	/* Sinlge argument given */
	for ( size_t i=0; i<sizeof(two_args_checks)/sizeof(two_args_checks[0]); i++ ) {
	    
	    struct string_method_pair arg_check = two_args_checks[i];
	    if ( !strcmp(argv[1], arg_check.string) || argv[1][0] == arg_check.string[0] ) {
		method = arg_check.method;
		break;
	    }
	}
    } else if ( argc == 3 ) {	/* 2 arguments given */
	for ( size_t i=0; i<sizeof(three_args_checks)/sizeof(three_args_checks[0]); i++ ) {
	    
	    struct string_method_pair arg_check = three_args_checks[i];
	    if ( !strcmp(argv[1], arg_check.string) || argv[1][0] == arg_check.string[0] ) {
		method = arg_check.method;
		break;
	    }
	}
    } else {			/* Too many arguments */
        printf("\t** Invalid argument combination **\n");
        method = broken;
    }
    return method;
}

/* Prints the result of the user asking for help or using the command line args incorrectly. */
void
workouts_handle_create_help_broken_methods(struct bus *mainbus)
{
    if ( mainbus->method == create )
    {
        workouts_create_workout(mainbus);
        mainbus->method = show;
        exit(EXIT_SUCCESS);
    } else if ( mainbus->method == help || mainbus->method == broken) {
        puts("usage:  workouts [option]");
        puts("___options___");
        printf("%-12s --  %s\n", "show", "Default, list all active, recent workouts and ids");
        printf("%-12s --  %s\n", "all", "Show all active and disactive, recent workouts");
        printf("%-12s --  %s\n", "create", "Create a new workout and add to active workouts");
        printf("%-12s --  %s\n", "help", "Display this help message");
        printf("%-9s %s --  %s\n", "progress", "id", "Advance an active workout to its next form");
        printf("%-9s %s --  %s\n", "edit", "id", "Edit any recent workout in-place");
        printf("%-9s %s --  %s\n", "rm", "id", "Deactivate an active workout");
        printf("%-9s %s --  %s\n", "list", "id", "Show history of one particular workout");
        exit(EXIT_SUCCESS);
    }
}


/* Creates a new workout  */
int
workouts_create_workout(struct bus *mainbus)
{
    // generate the new workout from user input
    struct workout workout_to_add = workouts_generate_workout(NULL);

    // open file for appending
    mainbus->workoutFile = workouts_safe_open_workoutfile_append(mainbus);

    // write the new workout
    workouts_write_full_workout(mainbus, workout_to_add);

    // close file
    workouts_safe_close_workoutfile(mainbus);

    // reassure user
    printf("'%s' added to workouts.\n", workout_to_add.exercise);
    return EXIT_SUCCESS;
}


/* Writes a workout to the workout file */
int
workouts_write_full_workout(struct bus *mainbus, struct workout workout_to_add)
{
    char *workout_string = workout_to_string(workout_to_add);
    fputs(workout_string, mainbus->workoutFile);
    free(workout_string);
    return EXIT_SUCCESS;
}


/* Opens the workout file for reading with errors if it fails to open */
FILE *
workouts_safe_open_workoutfile(struct bus *mainbus)
{
    FILE *workoutFile;
    if ( (workoutFile = fopen(mainbus->filename, "r")) == NULL ) {
        perror("Error opening workouts file");
        exit(EXIT_FAILURE);
    }
    return workoutFile;
}


/* Opens the workout file for appending with errors if it fails to open */
FILE *
workouts_safe_open_workoutfile_append(struct bus *mainbus)
{
    FILE *workoutFile;
    if ( (workoutFile = fopen(mainbus->filename, "a")) == NULL ) {
        perror("Error opening workouts file");
        exit(EXIT_FAILURE);
    }
    return workoutFile;
}


/* Counts number of newlines in the file as a safe (over)estimate of the number of workouts */
size_t
workouts_get_num_workouts(struct bus *mainbus)
{
    char c;
    size_t count = 0;
    for (c = getc(mainbus->workoutFile); c != EOF; c = getc(mainbus->workoutFile)) {
        if (c == '\n') {
            count++;
        }
    }
    // reset pointer to start of file
    if ( fseek(mainbus->workoutFile, 0, SEEK_SET) ) {
        perror("Error returning to start of file");
        exit(EXIT_FAILURE);
    }
    return count;
}


/* Read all lines from the workout file into the bus
   Normal workouts are read in full, rm's just add a
   nonactive headstone workout */
void
workouts_read_workoutfile_into_bus(struct bus *mainbus)
{
    /* +1s account for potential added or modified workouts */
    mainbus->workouts = malloc((mainbus->num_workouts+1) * sizeof(*mainbus->workouts));
    mainbus->recent_workouts = malloc((mainbus->num_workouts+1) * sizeof(*mainbus->recent_workouts));
    char buffer[MAX_WORKOUT_SIZE];
    size_t i = 0;
    while ( fgets(buffer, MAX_WORKOUT_SIZE, mainbus->workoutFile) ) {
	buffer[strlen(buffer)-1] = '\0'; /* Remove \n at end of each line */
	struct workout buffer_workout = string_to_workout(buffer);
	mainbus->workouts[i] = buffer_workout;
	workouts_update_recent_workouts(mainbus, mainbus->workouts[i]);
	i++;
    }
    return;
}


/* If workout is active (i.e. not a rm entry), check for it in recent_workouts
   and if it's there, update the pointer. If not, add it to the list. If it is
   not active (rm entry), remove the active status from the recent_workouts entry. */
void
workouts_update_recent_workouts(struct bus *mainbus, struct workout workout)
{
    /* Find the index of the entry in recent_workouts that matches input workout */
    int match_i = -1;
    for (size_t i=0; i < mainbus->num_uniques; i++) {
        if( !strcmp(mainbus->recent_workouts[i].id, workout.id) ) {
            match_i = i;
        }
    }
    if ( match_i == -1 ) {	/* Workout not found in recents */
        mainbus->recent_workouts[mainbus->num_uniques++] = workout;
    } else if ( workout.active ) { /* Workout found and active */
        mainbus->recent_workouts[match_i] = workout;
    } else {			/* workout found and rm entry */
	mainbus->recent_workouts[match_i].active = false;
    }
    return;
}


/* Returns the index of the most recent (latest in list) workout with id matching
   the given exercise */
size_t
workouts_get_most_recent_workout(struct bus *mainbus, char *exercise, size_t max_i)
{
    char id[7];
    workouts_get_id(exercise, id);
    for (int i=max_i; i>=0; i--) {
        if( !strcmp(mainbus->workouts[i].id, id) ) {
	    return i;
        }
    }
    fprintf(stderr, "Failed to find instance of workout id, %s for exercise %s.\n", id, exercise);
    return EXIT_FAILURE;
}


/* Takes a string "name," performs an algorithm, and puts the result into "id"
   Currently takes first letters of each word and then more if neccessary from the last word to make 4 chars */
void
workouts_get_id(char *name, char *id)
{
    char c = 'a';
    size_t leader_char_index = 0;
    size_t char_index = 0;
    size_t name_index = 0;
    while ((c = name[name_index++]) != '\0') {
        if (c == ' ' || c == '-') {
            leader_char_index++;
            char_index = 0;
        } else if ((leader_char_index + char_index) <= 3) {
            id[(leader_char_index + char_index++)] = tolower(c); 
        }
    }
    id[leader_char_index + char_index]= '\0';
    return;
}


/* Close the workout file with error reporting */
void
workouts_safe_close_workoutfile(struct bus *mainbus)
{
   if ( fclose(mainbus->workoutFile) ) {
        perror("Error closing workouts file");
        exit(EXIT_FAILURE);
    }
    return;
}


/* The final printing step for each of the cases */
void
workouts_print_workouts(struct bus *mainbus)
{
    /* TODO Figure out/implement how I want to sort the workouts (by date?) */
    // qsort(mainbus->recent_workouts_indexes, mainbus->num_uniques, sizeof(size_t), compare_size_t);

    workout_pprint_header();
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
int compare_size_t(const void *a, const void *b)
{
    return *(size_t *)a - *(size_t *)b;    
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
    mainbus->workoutFile = workouts_safe_open_workoutfile_append(mainbus);

    size_t most_current_index = workouts_get_most_recent_workout(mainbus, id, mainbus->num_workouts-1);

    struct workout temp_workout = mainbus->workouts[most_current_index];

    // fill fields with proper defaults
    struct split_string default_workout_ss = workout_to_split_string(temp_workout);
    char **default_workout = default_workout_ss.str_p_array;
    char *todays_date = workouts_get_todays_date();
    
    default_workout[5] = todays_date;

    temp_workout = workouts_generate_workout(default_workout);

    // Then write full
    workouts_write_full_workout(mainbus, temp_workout);
    mainbus->workouts[mainbus->num_workouts] = temp_workout;
    workouts_update_recent_workouts(mainbus, mainbus->workouts[mainbus->num_workouts]);

    free_split_string(default_workout_ss);
    free(todays_date);

    // close file
    workouts_safe_close_workoutfile(mainbus);
    return EXIT_SUCCESS;
}


char *
workouts_get_todays_date(void)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char *todays_date = malloc(7);
    strftime(todays_date, 7, "%y%m%d", tm);
    return todays_date;
}


int
workouts_rm_wid_workout(struct bus *mainbus, char *id)
{
    // open file for appending
    mainbus->workoutFile = workouts_safe_open_workoutfile_append(mainbus);

    // get matching workout
    size_t most_current_index = workouts_get_most_recent_workout(mainbus, id, mainbus->num_workouts-1);
    struct workout temp_workout = mainbus->workouts[most_current_index];

    // Then write rm field
    workouts_write_rm_workout(mainbus->workoutFile, temp_workout);
    // Update mainbus
    workouts_update_recent_workouts(mainbus, temp_workout);

    // close file
    workouts_safe_close_workoutfile(mainbus);
    return EXIT_SUCCESS;
}


int
workouts_write_rm_workout(FILE* workoutFile, struct workout workout_to_add)
{
    fprintf(workoutFile, "%s|%s\n", workout_to_add.exercise, "rm");
    return EXIT_SUCCESS;
}


int
workouts_list_wid_workout(struct bus *mainbus, char *id)
{
    // print all matching workouts
    workout_pprint_header();
    for (size_t i=0; i < mainbus->num_workouts; i++) {
        if( !strcmp(mainbus->workouts[i].id, id) ) {
	    workout_pprint(mainbus->workouts[i]);
        }
    }
    exit(EXIT_SUCCESS);
}


void
workouts_edit_wid_workout(struct bus *mainbus, char *id)
{
    // open temp file for writing and main file for reading
    struct bus tempbus = bus_default;
    tempbus.filename = "tmp_workoutseditfile.txt";
    tempbus.workoutFile = workouts_safe_open_workoutfile_append(&tempbus);
    mainbus->workoutFile = workouts_safe_open_workoutfile(mainbus);

    size_t most_current_index = workouts_get_most_recent_workout(mainbus, id, mainbus->num_workouts-1);
    struct workout temp_workout = mainbus->workouts[most_current_index];
    struct workout generated_workout;

    // fill fields with proper defaults
    struct split_string default_workout_ss = workout_to_split_string(temp_workout);
    char **default_workout = default_workout_ss.str_p_array;
    generated_workout = workouts_generate_workout(default_workout);

    // Then write whole file replacing just the edited workout
    workouts_write_edited_workout(mainbus, &tempbus, temp_workout, generated_workout);

    free_split_string(default_workout_ss);

    // close files
    workouts_safe_close_workoutfile(mainbus);
    workouts_safe_close_workoutfile(&tempbus);

    // move temp file to overwrite real one
    rename(tempbus.filename, mainbus->filename);

    return;
}


int
workouts_write_edited_workout(struct bus *mainbus, struct bus *tempbus, struct workout original_workout, struct workout edited_workout)
{
    char *workout_line;
    struct workout woi;
    for ( size_t i=0; i<mainbus->num_workouts; i++ ) {
	woi = mainbus->workouts[i];
	if ( workout_compare(woi, original_workout) ) /* If they're the same */
        {
	    /* Replace the workout in file and in bus */
            workouts_write_full_workout(tempbus, edited_workout);
	    mainbus->workouts[i] = edited_workout;
        } else {
            // write previous
	    workout_line = workout_to_string(edited_workout);
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

    char id[7];
    workouts_get_id(workout_pointers[0], id);
    generated_workout.id = strdup(id);
    generated_workout.exercise = workout_pointers[0];
    generated_workout.weights = workout_pointers[1];
    generated_workout.sets = workout_pointers[2];
    generated_workout.reps = workout_pointers[3];
    generated_workout.days = workout_pointers[4];
    generated_workout.date = workout_pointers[5];
    generated_workout.notes = workout_pointers[6]; 
    
    // uncomment to see the whole allocated buffer in hex
    // for (int i=0; i<525; i++) { printf("%#.2x ", *(*total_buffer + i)); if ((i+1)%50 == 0) printf("\n"); } printf("\n");
    return generated_workout;
}
