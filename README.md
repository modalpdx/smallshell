# Small Shell
A school project for a small "shell" with threee built-in commands
and lots of fork/exec goodness.

##How the small shell works:

I'll start by stating that this is not a full-featured login shell.
Essentially, it gives you a command prompt (":") where you can execute any
program on your computer or one of the three built-in functions:

- cd: Like the normal change-directory command on Linux, but without bells
  and whistles. If you just run 'cd' by itself it sends you to your home
  directory. If you specify a path after 'cd' you will change to that
  directory, assuming it exists.

- status: Displays the exit status code for the last command/program that
  was executed.

- exit: Exits the small shell.

This assignment was an exercise in UNIX signal handling and
forking/execing processes. From the end-user standpoint, it's not
exciting. The code is more interesting than the action.

##How this program works:

Just run 'smallsh' at a UNIX command prompt. The small shell will start,
you'll be offered a ":" command prompt, and you'll stay in the shell until
you execute the 'exit' command. That's it.

##Build:

Download everyting and run 'make'. There is no command line help because
there are no command line options. Just run 'smallsh' to make it go.

##Colophon:

This program was written with standards in mind but was only
developed and tested on Linux and Mac OS. Mostly Linux.

GNU Make and GCC are required. The program is 100% C, no C++ or other C
variants.

##Disclaimer:

This is code from a school project. It satisfies assignment requirements
but is nowhere near as "scrubbed" as released software should be.
Security is not addressed, only functionality and no input
validation. If you use this code for anything other than satisfying your
curiosity, please keep the following in mind:

- there is no warranty of any kind (you use the code as-is)
- there is no support offered, please do not ask
- there is no guarantee it'll work, although it's not complex so it should
  work
- please do not take credit for code you did not write, especially if you
  are a student. NO CHEATING.

Thanks!
