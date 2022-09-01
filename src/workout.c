/* Defines functions that act on the workout struct */

#include <string.h>
#include "workouts.h"


char *
workout_to_string(struct workout workout_in)
{
    char *buffer = malloc(7 + sizeof(struct workout));
    sprintf(buffer, "%s|%s|%s|%s|%s|%s\n", workout_in.exercise, workout_in.weights, workout_in.sets, workout_in.reps, workout_in.date, workout_in.notes);
    return buffer;
}


struct workout
string_to_workout(char* string)
{
    char id[10];
    struct split_string parsed_string = strsplit(string, '|');
    workouts_get_id(parsed_string.str_p_array[0], id);
    char **workout_fields = parsed_string.str_p_array;
    struct workout return_workout;

    /* If the length of the field is >= WORKOUT_FIELD_LENGTH, then there are issues with \0 termination with strncpy */
    return_workout.active = true;
    strncpy(return_workout.id, id, WORKOUT_FIELD_LENGTH);
    strncpy(return_workout.exercise, workout_fields[0], WORKOUT_FIELD_LENGTH);
    strncpy(return_workout.weights, workout_fields[1], WORKOUT_FIELD_LENGTH);
    strncpy(return_workout.sets, workout_fields[2], WORKOUT_FIELD_LENGTH);
    strncpy(return_workout.reps, workout_fields[3], WORKOUT_FIELD_LENGTH);
    strncpy(return_workout.date, workout_fields[4], WORKOUT_FIELD_LENGTH);
    strncpy(return_workout.notes, workout_fields[5], 4*WORKOUT_FIELD_LENGTH);
    return return_workout;
}
