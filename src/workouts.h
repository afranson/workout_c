// .h file for the whole workouts program collective

#ifndef WORKOUTS_H
#define WORKOUTS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define WORKOUT_FIELD_LENGTH 50

enum methods { broken, show, create, progress, progress_wid, edit, edit_wid, rm, rm_wid, list, list_wid, all, help };

struct workout
{
    bool active;
    char id[WORKOUT_FIELD_LENGTH];
    char exercise[WORKOUT_FIELD_LENGTH];
    char weights[WORKOUT_FIELD_LENGTH];
    char sets[WORKOUT_FIELD_LENGTH];
    char reps[WORKOUT_FIELD_LENGTH];
    char days[WORKOUT_FIELD_LENGTH];
    char date[WORKOUT_FIELD_LENGTH];
    char notes[4*WORKOUT_FIELD_LENGTH];
};

struct workouts
{
    struct workout *all_lines; 	/* Contains the whole file */
    struct workout *all_workouts; /* Gets rid of the rm's */
    struct workout *recent_workouts; /* Only the active and most recent version of workouts */
};

struct split_string
{
    char **str_p_array;
    size_t num_elements;
};

struct bus
{
    int argc;
    char **argv;
    enum methods method;
    char *filename;
    FILE *workoutFile;
    size_t num_workouts;
    struct workout *workouts;
    size_t *recent_workouts_indexes;
    size_t num_uniques;
};

extern struct workout workout_default;
extern struct bus bus_default;

/* Utilities */
void print_buffer(char* buff, size_t len);

/* Splitting Strings */
struct split_string strsplit(char * string_to_split, char delim);
void free_split_string(struct split_string);
void print_split_string(struct split_string);
int test_strsplit(void);

/* Workouts Functions */
struct bus workouts_init_bus(int, char **, char *);
enum methods workouts_parse_options(struct bus *);
void workouts_handle_create_help_broken_methods(struct bus *);
 int workouts_create_workout(struct bus *);
  int workouts_write_full_workout(struct bus *, struct workout);
FILE *workouts_safe_open_workoutfile(struct bus *);
FILE *workouts_safe_open_workoutfile_append(struct bus *);
size_t workouts_get_num_workouts(struct bus *);
void workouts_read_workoutfile_into_bus(struct bus *);
 int workouts_read_rmline(struct bus *, struct split_string);
 int workouts_read_fullline(struct bus *, size_t i, struct split_string);
 void workouts_update_recent_workouts_indexes(struct bus *, size_t);
 size_t workouts_get_most_recent_workout(struct bus *, char *);
  void workouts_get_id(char *, char *);
void workouts_safe_close_workoutfile(struct bus *);
void workouts_print_workouts(struct bus *);
 size_t *workouts_get_indexes_to_print(struct bus *, size_t *);
 int compare_size_t(const void *, const void *);
void workouts_wid_actions(struct bus *);
 void workouts_progress_wid_workout(struct bus *, char *);
  char *workouts_get_todays_date(void);
 void workouts_rm_wid_workout(struct bus *, char *);
  int workouts_write_rm_workout(FILE *, struct workout);
 void workouts_list_wid_workout(struct bus *, char *);
 void workouts_edit_wid_workout(struct bus *, char *);
  int workouts_write_edited_workout(struct bus *, struct bus *, struct workout, struct workout);
  int workouts_cmp_line_to_workout(char *, struct workout);
  struct workout workouts_generate_workout(char *[6]);
  char **workouts_workout_to_charss(struct workout *);

#endif
