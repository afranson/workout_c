# Workouts

A simple command line interface (CLI) for recording and viewing workouts.

The goal here is to provide a robust way of storing workout information over time with an easy-to-use interface for adding new workouts, removing workouts, progressing workouts, and viewing workouts.

## Basic Usage

Viewing workouts/options:
```
cd /path/to/git_repo
./bin/workouts s     # Show current workouts
./bin/workouts a     # Show the most recent version of all workouts in file
./bin/workouts h     # Show all options and usage
./bin/workouts l id  # List all workouts with matching id
./bin/workouts d id  # Show most recent workout matching id in full detail
```

Modifying workouts:
```
cd /path/to/git_repo
./bin/workouts c     # Create a brand new workout
./bin/workouts p id  # Progress a workout with matching id (add a new entry that is a derivative of the last one)
./bin/workouts r id  # Take a workout (id) out of the current list of workouts
./bin/workouts e id  # Edit the most recent workout with matching id
```

Backup workouts:
```
./bin/workouts b     # Creates a backup of the workoutinfo.txt file in gzip format
```

Searching workouts:
```
./bin/workout a | fzf
# ^ Use your searcher of choice (grep, sed, fzf, etc.) to find that one workout you did for a month two years ago that started with a p and had two t's

alias wod='date; /path/to/git_repo/bin/workouts | grep -E "(Days|$(date | cut -c 1-3))"'
# ^ See all of the workouts slotted for today (assuming you use 3+ letter abbreviations [Mon, Tue, ...] for the "Days" field)

wods () {
    date;
    if [ $# -eq 0 ]; then
	/home/aj/Documents/workout_progn/workout_c/bin/workouts | grep -E "(Days|$(date | cut -c 1-3))"
    elif [ $# -eq 1 ]; then
	/home/aj/Documents/workout_progn/workout_c/bin/workouts | grep -E "(Days|$1)"
    elif [ $# -eq 2 ]; then
	/home/aj/Documents/workout_progn/workout_c/bin/workouts | grep -E "(Days|$1|$2)"
    elif [ $# -eq 3 ]; then
	/home/aj/Documents/workout_progn/workout_c/bin/workouts | grep -E "(Days|$1|$2|$3)"
    else
	echo "Too many args"
    fi
}
# ^ Bash function to show what workouts you have to do (0 args = today, 1-3 args = on those days)
```

## Install

Clone the repo, enter the base directory, 'make clean' it, remove the default workout entries, and run 'make'. The workouts datafile is in data/, which must currently sit in bin/../data (data/ must be in same dir as bin/). Alias to the bin/ directory and enjoy.

```
git clone https://github.com/afranson/workouts
cd workouts
make clean
rm data/workoutinfo.txt data/workoutinfo.txt.full
make
```
You may need to install 'make' and 'gcc' depending on your distro and prior install history.

## Tutorial
```
# Tutorial
./bin/workouts c   # Create your first workout entry
./bin/workouts s   # Show the entry
./bin/workouts p your_id   # Progress the workout you just created
```
