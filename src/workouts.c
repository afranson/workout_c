#include <stdio.h>
#include <stdlib.h>
#include <libgen.h> 		/* basename, dirname */
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include "workouts.h"

/* How workouts are printed in the terminal */
const char *workout_pprint_format = "%-4s |%-30s |%-10s |%-4s |%-7s |%-8s |%-40s\n";

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
    fprintf(mainbus->workoutFile, "%s|%s|%s|%s|%s|%s\n", workout_to_add.exercise, workout_to_add.weights, workout_to_add.sets, workout_to_add.reps, workout_to_add.date, workout_to_add.notes);
    return 0;
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
   If a line has 2 entries, it's probably removed a workout
   If it has 6 entries, it's probably a properly formatted line */
void
workouts_read_workoutfile_into_bus(struct bus *mainbus)
{
    mainbus->workouts = malloc(mainbus->num_workouts * sizeof(struct workout));
    mainbus->recent_workouts_indexes = malloc(mainbus->num_workouts * sizeof(size_t));
    size_t workout_size = MAX_WORKOUT_SIZE; /* Long workout line */
    char buffer[workout_size];
    size_t i = 0;
    while ( fgets(buffer, workout_size, mainbus->workoutFile) )
    {
	buffer[strlen(buffer)-1] = '\0'; /* Remove \n at end of each line */
        struct split_string parsed_string = strsplit(buffer, '|');
        if (parsed_string.num_elements == 2 && !strcmp(parsed_string.str_p_array[1], "rm") ) {
            workouts_read_rmline(mainbus, parsed_string, i);
            i--; // don't advance counter
            mainbus->num_workouts--;
        } else if (parsed_string.num_elements == 6) {
            workouts_read_full_line(mainbus, i, buffer);
            workouts_update_recent_workouts_indexes(mainbus, i);
        } else {
            perror("Error importing workout line from file");
        }
        free_split_string(parsed_string);
        i++;
    }
    return;
}


int
workouts_read_rmline(struct bus *mainbus, struct split_string parsed_string, size_t i)
{
    // remove active stat from most recent workout matching parsed_string[0]
    size_t workout_i = workouts_get_most_recent_workout(mainbus, parsed_string.str_p_array[0], i-1);
    mainbus->workouts[workout_i].active = false;
    return EXIT_SUCCESS;
}


int
workouts_read_full_line(struct bus* mainbus, size_t i, char* string)
{
    struct workout this_workout = string_to_workout(string);
    mainbus->workouts[i] = this_workout;

    return EXIT_SUCCESS;
}


void
workouts_update_recent_workouts_indexes(struct bus *mainbus, size_t i)
{
    size_t match_j = 0;
    for (size_t j=0; j < mainbus->num_uniques; j++) {
        if( !strcmp(mainbus->workouts[mainbus->recent_workouts_indexes[j]].id, mainbus->workouts[i].id) ) {
            match_j = j;
        }
    }
    if ( match_j == 0 ) {
        mainbus->recent_workouts_indexes[mainbus->num_uniques] = i;
        mainbus->num_uniques++;
    } else {
        mainbus->recent_workouts_indexes[match_j] = i;
    }
    return;
}


/* Returns the index of the most recent (latest in list) workout with id matching
   the given exercise */
size_t
workouts_get_most_recent_workout(struct bus *mainbus, char *exercise, size_t max_i)
{
    char id[10];
    workouts_get_id(exercise, id);
    for (int i=max_i; i>=0; i--) {
        if( !strcmp(mainbus->workouts[i].id, id) ) {
	    return i;
        }
    }
    fprintf(stderr, "Failed to find instance of workout id, %s.\n", id);
    return -1;
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
    // get the fields to be printed for the first round of possible printing
    qsort(mainbus->recent_workouts_indexes, mainbus->num_uniques, sizeof(size_t), compare_size_t);

    printf(workout_pprint_format, "id", "Exercise", "Weights", "Sets", "Reps", "Date", "Notes");
    for (size_t i=0; i<mainbus->num_uniques; i++) {
        size_t index = mainbus->recent_workouts_indexes[i];
        switch ( mainbus->method ) {
        case all:
        case list:
        case rm:
        case progress:
        case edit:
            printf(workout_pprint_format, mainbus->workouts[index].id, mainbus->workouts[index].exercise, mainbus->workouts[index].weights, mainbus->workouts[index].sets, mainbus->workouts[index].reps, mainbus->workouts[index].date, mainbus->workouts[index].notes);
            break;
        case show:
            if ( mainbus->workouts[index].active ) {
                printf(workout_pprint_format, mainbus->workouts[index].id, mainbus->workouts[index].exercise, mainbus->workouts[index].weights, mainbus->workouts[index].sets, mainbus->workouts[index].reps, mainbus->workouts[index].date, mainbus->workouts[index].notes);
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


void 
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
    
    default_workout[4] = todays_date;

    temp_workout = workouts_generate_workout(default_workout);
    printf("here?\n");
    // Then write full
    workouts_write_full_workout(mainbus, temp_workout);

    free_split_string(default_workout_ss);

    // close file
    workouts_safe_close_workoutfile(mainbus);
    exit(EXIT_SUCCESS);
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


void
workouts_rm_wid_workout(struct bus *mainbus, char *id)
{
    // open file for appending
    mainbus->workoutFile = workouts_safe_open_workoutfile_append(mainbus);

    // get matching workout
    size_t most_current_index = workouts_get_most_recent_workout(mainbus, id, mainbus->num_workouts-1);
    struct workout temp_workout = mainbus->workouts[most_current_index];

    // Then write rm field
    workouts_write_rm_workout(mainbus->workoutFile, temp_workout);

    // close file
    workouts_safe_close_workoutfile(mainbus);
    exit(EXIT_SUCCESS);
}


int
workouts_write_rm_workout(FILE* workoutFile, struct workout workout_to_add)
{
    fprintf(workoutFile, "%s|%s\n", workout_to_add.exercise, "rm");
    return 0;
}


void
workouts_list_wid_workout(struct bus *mainbus, char *id)
{
    // print all matching workouts
    printf(workout_pprint_format, "id", "Exercise", "Weights", "Sets", "Reps", "Date", "Notes");
    for (size_t i=0; i < mainbus->num_workouts; i++) {
        if( !strcmp(mainbus->workouts[i].id, id) ) {
            printf(workout_pprint_format, mainbus->workouts[i].id, mainbus->workouts[i].exercise, mainbus->workouts[i].weights, mainbus->workouts[i].sets, mainbus->workouts[i].reps, mainbus->workouts[i].date, mainbus->workouts[i].notes);
        }
    }
    exit(EXIT_SUCCESS);
}


void 
workouts_edit_wid_workout(struct bus *mainbus, char *id)
{
    // open temp file for writing and main file for reading
    struct bus tempbus = bus_default;
    tempbus.filename = strdup("tmp_workoutseditfile.txt");
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

    /* TODO need to free the tempbus */
    return;
}


int
workouts_write_edited_workout(struct bus *mainbus, struct bus *tempbus, struct workout original_workout, struct workout edited_workout)
{
    char buffer[MAX_WORKOUT_SIZE];
    while ( fgets(buffer, MAX_WORKOUT_SIZE, mainbus->workoutFile ) ) {
        if ( !workouts_cmp_line_to_workout(buffer, original_workout) )
        {
            //write new thing
            workouts_write_full_workout(tempbus, edited_workout);
        } else {
            // write old thing
            fprintf(tempbus->workoutFile, "%s", buffer);
        }
    }
    return 0;
}


int
workouts_cmp_line_to_workout(char *buffer, struct workout active_workout)
{
    struct split_string ss = strsplit(buffer, '|');
    char **struct_list = ss.str_p_array;

    if ( ss.num_elements == 2 ) {
        return 1;
    } else if ( !strcmp(struct_list[0], active_workout.exercise)
                && !strcmp(struct_list[1], active_workout.weights)
                && !strcmp(struct_list[2], active_workout.sets)
                && !strcmp(struct_list[3], active_workout.reps)
                && !strcmp(struct_list[4], active_workout.date)
                && !strcmp(struct_list[5], active_workout.notes) ) {
        return 0;
    } else {
        return 1;
    }
}

/* TODO this could be a lot better */
struct workout
workouts_generate_workout(char **default_options)
{
    /* TODO maybe ncurses to prefill stdin with default values */
    // add something in
    // allocate new workout space
    char **total_buffer = (char **)malloc( MAX_WORKOUT_SIZE*sizeof(char *));
    char *first_data_loc = (char *)(total_buffer + 6);
    memset(total_buffer, 0, 6*sizeof(char *) + MAX_WORKOUT_SIZE);
    for (int i=0; i<6; i++) {
        total_buffer[i] = first_data_loc + 50*i;
    }

    // collect fields from user
    const char *fields[] = {"Name", "Weights", "Sets", "Reps", "Date", "Notes"};
    printf("Enter Name then Weights ... Sets Reps Date Notes\n");
    if ( default_options == NULL ) {
        for (int i=0; i<6; i++)
        {
            printf("%s: ", fields[i]);
	    fgets(total_buffer[i], 200, stdin);
        }
    } else {
        for (int i=0; i<6; i++)
        {
            printf("%s [%s]: ", fields[i], default_options[i]);
            fgets(total_buffer[i], 200, stdin);
            if ( total_buffer[i][0] == '\n' || !total_buffer[i] ) {
                strcpy(total_buffer[i], default_options[i]);
            }
        }

    }

    // remove \n from the ends
    int j;
    char c;
    for (int i=0; i<6; i++)
    {
        j=0;
        do {
            c = total_buffer[i][j];
            j++;
        } while ( c != '\n' && c != '\0' );
        total_buffer[i][j-1] = '\0';
    }

    // uncomment to see the whole allocated buffer in hex
    // for (int i=0; i<525; i++) { printf("%#.2x ", *(*total_buffer + i)); if ((i+1)%50 == 0) printf("\n"); } printf("\n");
    struct workout workout_to_add = workout_default;
    workout_to_add.exercise = total_buffer[0];
    workout_to_add.weights = total_buffer[1];
    workout_to_add.sets = total_buffer[2];
    workout_to_add.reps = total_buffer[3];
    workout_to_add.date = total_buffer[4];
    workout_to_add.notes = total_buffer[5];
    return workout_to_add;
}
