README file for the Assignment 3 - smallsh

Author: Max Grier
Data: 02-08-21
Project: Assignment 3 - smallsh

Included files:
    main.c
        This file has the majority of the functionality and program flow for the program.
    main_helpers.h
	This has a lot of the helper functions that are called by main.c to run the program.  This helps to break out the code into compartments, making main.c easier to follow and the functions easier to edit.
    main_sigHandler.h
	Similar to main_helpers.h, this file has helper functions for sigaction.  These are called by main.c to handle sigaction structs.

How to compile:
    For this program, we are utilizing the gcc compiler.
    To compile main.c, you will need to be in the directory where the main.c file resides,
    then enter the following command in your terminal:
        gcc --std=gnu99 -o smallsh main.c

How to run to code:
    Once you have the executable file, "smallsh", you are ready to use the program.
    Run the following command in your terminal window:
        ./smallsh
        Note: if you want to use the test script, you will run this command:
              ./p3testscript > mytestresults 2>&1
	      Then the results will be in mytesteresults

How to use the program:
    Once you have compiled and started running the program, it will prompt you with 
    ": " and act just like a shell or terminal.
    You can run commands as you would in a shell window (ls, status, exit, etc.).
    You can also run processes in the background by entering "&" at the end of your
    command line.


