/* Defines functions that act on the workout struct */

#include <string.h>
#include "workouts.h"

/* Trying to construct things atomistically */

size_t
workout_get_num_chars(struct workout workout_in)
{
    /* Need to add in days later */
    return strlen(workout_in.id) + strlen(workout_in.exercise) + strlen(workout_in.weights) + strlen(workout_in.sets) + strlen(workout_in.reps) + strlen(workout_in.days) + strlen(workout_in.date) + strlen(workout_in.notes);
}


char *
workout_to_string(struct workout workout_in)
{
    /* Upgrade to snprintf for safety */
    char *buffer = malloc(8 + sizeof(char)*workout_get_num_chars(workout_in));
    sprintf(buffer, "%s|%s|%s|%s|%s|%s|%s\n", workout_in.exercise, workout_in.weights, workout_in.sets, workout_in.reps, workout_in.days, workout_in.date, workout_in.notes);
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


struct workout
string_to_workout(char* string)
{
    char *id = malloc(5 * sizeof(char));
    struct split_string parsed_string = strsplit(string, '|');
    workouts_get_id(parsed_string.str_p_array[0], id);
    char **workout_fields = parsed_string.str_p_array;
    struct workout return_workout;

    return_workout.active = true;
    return_workout.id = id;
    return_workout.exercise = workout_fields[0];
    return_workout.weights = workout_fields[1];
    return_workout.sets = workout_fields[2];
    return_workout.reps = workout_fields[3];
    return_workout.days = workout_fields[4];
    return_workout.date = workout_fields[5];
    return_workout.notes = workout_fields[6];
    return return_workout;
}


/* void */
/* add_workout_to_workouts(struct workouts wos, struct workout wo) */
/* { */
/*     return; */
/* } */
