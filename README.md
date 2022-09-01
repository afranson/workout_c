# Workouts

A simple, lightweight command line interface (CLI) for recording workouts.

The goal here is to have a simple, robust way of storing workout information over time with an easy-to-use interface for adding new workouts, removing workouts, progressing workouts, viewing workouts, etc.


# Stuff to do
- sorting options (last date, alphabetical, any others?)
- user can use id to skip the asking phase
  - -> workouts progress 4c ==> name as first 3 letters then a number
  - workouts progress squ1 or dea1 or rom2 or tri5 or spl3 -- shouldn't need more than 9 options for any given 3 letters right?
- workout planning is a completely different beast that needs a different tool
- can I add basic workout tracking though?
  - volume, weight, intensity over time
- preprogram workouts?
  - that's what you need for mass appeal
  - how would - Bicep Curl on Mon, Wed, Fri show up?
  - you'd have to see the date, check if it fits, then show what's happening for today
  - you'd still have to manually input dates and weights and reps and stuff - anything automatic like that requires progamming particular plans sets in
  - if I could add a calculator / automatic programmer, that would be pretty cool
  - just reaches out for a month, then deload or remake if you want to continue
    - just need start and end reps, weight, sets - puts the program in the the user just has to modify it as they go along
    - much easier to do in a spreadsheet, this is not the target audience
- allow to add color to special ones (make squats blue or biceps red or whatever you're most interested in that cycle)
- if you edit the name field, it must go back and account for this everywhere
  - could ask to create new entry or replace preexisting one
- don't allow option to modify name
