/* Defines functions that act on the workout struct */

#include <string.h>
#include <ctype.h>
#include "workouts.h"


/* TODO Can I create a tuple structure to retain the named aspect and be able to loop? */

/* Trying to construct things atomistically */

const char *workout_pprint_format = "%-6.6s |%-25.25s |%-9.9s |%-4.4s |%-6.6s |%-11.11s |%-8.8s |%-.40s\n";
const char *workout_detail_format = "ID: %s\nExercise: %s\nWeight: %s\nSets: %s\nReps: %s\nDays: %s\nDate: %s\nNotes: %s\n";


void
workout_pprint_header()
{
    printf(workout_pprint_format, "id", "Exercise", "Weights", "Sets", "Reps", "Days", "Date", "Notes");
}


void
workout_pprint(struct workout workout)
{
    printf(workout_pprint_format, workout.id, workout.exercise, workout.weights, workout.sets, workout.reps, workout.days, workout.date, workout.notes);
}


void
workout_detail_print(struct workout workout)
{
    printf(workout_detail_format, workout.id, workout.exercise, workout.weights, workout.sets, workout.reps, workout.days, workout.date, workout.notes);
}


size_t
workout_get_num_chars(struct workout workout)
{
    return strlen(workout.exercise) + strlen(workout.weights) + strlen(workout.sets) + strlen(workout.reps) + strlen(workout.days) + strlen(workout.date) + strlen(workout.notes);
}


char *
workout_to_string(struct workout workout)
{
    /* Upgrade to snprintf for safety */
    char *buffer = malloc(8 + sizeof(char)*workout_get_num_chars(workout));
    if ( !strcmp(workout.notes, "rm") ) { // It's an rm workout
	sprintf(buffer, "%s|rm\n", workout.exercise);
    } else {
	sprintf(buffer, "%s|%s|%s|%s|%s|%s|%s\n", workout.exercise, workout.weights, workout.sets, workout.reps, workout.days, workout.date, workout.notes);
    }
    return buffer;
}


struct split_string
workout_to_split_string(struct workout workout_in)
{
    char *workout_string = workout_to_string(workout_in);
    workout_string[strlen(workout_string) - 1] = '\0';
    // remove_end_newline(&workout_string);
    struct split_string ss = strsplit(workout_string, '|');
    free(workout_string);
    return ss;
}


struct workout
create_rm_workout(char *exercise)
{
    struct workout return_workout;
    return_workout.active = false;
    return_workout.id = workout_get_id(exercise);
    return_workout.exercise = exercise;
    return_workout.notes = "rm";
    return return_workout;
}


/* Parses a line into a workout. If the line removes a workout from themselves
   active ppol of workouts, it is saved with the 'active' field set to false*/
struct workout
string_to_workout(char* string)
{
    struct split_string parsed_string = strsplit(string, '|');
    char **workout_fields = parsed_string.str_p_array;
    struct workout return_workout;

    if ( parsed_string.num_elements == 2 ) { // rm
	return_workout = create_rm_workout(workout_fields[0]);
    } else {
	return_workout.active = true;
	return_workout.id = workout_get_id(workout_fields[0]);
	return_workout.exercise = workout_fields[0];
	return_workout.weights = workout_fields[1];
	return_workout.sets = workout_fields[2];
	return_workout.reps = workout_fields[3];
	return_workout.days = workout_fields[4];
	return_workout.date = workout_fields[5];
	return_workout.notes = workout_fields[6];
    }
    free(workout_fields);
    return return_workout;
}


int
workout_compare(struct workout workout_a, struct workout workout_b)
{
    if ( !strcmp(workout_a.exercise, workout_b.exercise)
	 && !strcmp(workout_a.weights, workout_b.weights)
	 && !strcmp(workout_a.sets, workout_b.sets)
	 && !strcmp(workout_a.reps, workout_b.reps)
	 && !strcmp(workout_a.days, workout_b.days)	
	 && !strcmp(workout_a.date, workout_b.date)
	 && !strcmp(workout_a.notes, workout_b.notes) ) { // If all fields are equal
        return 1;
    } else {
	return 0;
    }
}


/* Takes a string "name," performs an algorithm, and puts the result into "id"
   Currently takes first letters of each word and then more if neccessary from the last word to make 4 chars */
char *
workout_get_id2(char *name)
{
    char *id = malloc(7 * sizeof(*id));
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
    return id;
}


/* Generates id by taking first 2 chars from each word (space sep) */
char *
workout_get_id(char *name)
{
    char *id = malloc(7 * sizeof(*id));
    memset(id, 0, 8);
    size_t chars_left_from_current_word = 2;
    size_t id_index = 0;
    for (size_t i=0; i<strlen(name); i++) { /* Loop over name */
	if (id_index == 6) {		    /* Filled the buffer */
	    return id;
	}
	if (chars_left_from_current_word > 0) { /* If we're taking chars */
	    id[id_index++] = tolower(name[i]);
	    chars_left_from_current_word--;
	} else if (name[i] == ' ') { /* Not taking chars, and rand into a space (indicating new word) */
	    chars_left_from_current_word = 2;
	}
    }
    return id;
}
