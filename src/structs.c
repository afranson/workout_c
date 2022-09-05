/* Defines functions that act on the workout struct */

#include <string.h>
#include "workouts.h"

/* Trying to construct things atomistically */

const char *workout_pprint_format = "%-4s |%-25s |%-9s |%-4s |%-6s |%-11s |%-8s |%s\n";


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
    sprintf(buffer, "%s|%s|%s|%s|%s|%s|%s\n", workout.exercise, workout.weights, workout.sets, workout.reps, workout.days, workout.date, workout.notes);
    return buffer;
}


struct split_string
workout_to_split_string(struct workout workout_in)
{
    char *workout_string = workout_to_string(workout_in);
    workout_string[strlen(workout_string)-1] = '\0'; /* remove final \n */
    struct split_string ss = strsplit(workout_string, '|');
    free(workout_string);
    return ss;
}


/* Parses a line into a workout. If the line removes a workout from themselves
   active ppol of workouts, it is saved with the 'active' field set to false*/
struct workout
string_to_workout(char* string)
{
    char *id = malloc(5 * sizeof(char));
    struct split_string parsed_string = strsplit(string, '|');
    workouts_get_id(parsed_string.str_p_array[0], id);
    char **workout_fields = parsed_string.str_p_array;
    struct workout return_workout;

    if ( parsed_string.num_elements == 2 ) { /* rm */
	return_workout.active = false;
	return_workout.id = id;
	return_workout.exercise = workout_fields[0];
    } else {
	return_workout.active = true;
	return_workout.id = id;
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
	 && !strcmp(workout_a.notes, workout_b.notes) ) {
        return 1;
    } else {
	return 0;
    }
}
