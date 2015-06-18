//
// *****************************************************************************
// 
// Author:    Erik Ratcliffe
// Date:      May 26, 2015
// Project:   Program 3 - smallsh
// Filename:  smallsh.h
// Class:     CS 344 (Spring 2015)
//
//
// Overview:
//    Basic shell with three built-in commands and basic signal handling.
//
//    This file contains variable definitions and function prototypes.
//
// *****************************************************************************
//


#ifndef SMALLSH_H
#define SMALLSH_H


#include <signal.h>


#define PROMPT    ": "          // Basic command prompt string
#define MAX_USER_INPUT 2048     // Maximum length of user input
#define MAX_ARGS  512           // Maximum number of arguments from the user


int pstatus; // holds whatever status happens to be the latest


// struct Node: Holds PID information for a background process
//
// pid  -> PID of the process, returned by fork()
//
// next -> The next node in the list
//
struct Node {
    pid_t pid;
    struct Node *next;
};


// *****************************************************************************
// 
// struct Node *addNode(int newPid, struct Node *head)
//
//    Entry:   int newPid
//                PID to add to the background process PID list
//             struct Node *head
//                Pointer to the head node of the list
//
//    Exit:    Returns a pointer to the new head node.
//
//    Purpose: Add a new background process PID to the background process PID list.
//
// *****************************************************************************
//
struct Node *addNode(int newPid, struct Node *head);


// *****************************************************************************
// 
// struct Node *clearChildren(struct Node *head, int *numNodes)
//
//    Entry:   struct Node *head
//                Pointer to the head node of the list
//             int *numNodes
//                Pointer to integer containing the number of nodes in the list.
//
//    Exit:    Returns the pointer to head, in case it is needed.
//
//    Purpose: Walk through a list of background PIDs and reap zombies.
//
// *****************************************************************************
//
struct Node *clearChildren(struct Node *head, int *numNodes);


// *****************************************************************************
// 
// void myCd(char *userArgs[], int numArgs)
//
//    Entry:   char *userArgs[]
//                Pointer array containing a NULL-terminated list of arguments.
//             int numArgs
//                Integer containing the number of command line arguments + NULL.
//
//    Exit:    None.
//
//    Purpose: Change to a directory specified by the user (or home directory
//             if no directory was specified).
//
// *****************************************************************************
//
void myCd(char *userArgs[], int numArgs);


// *****************************************************************************
// 
// void myStatus(int pstatus)
//
//    Entry:   int pstatus
//                Integer containing the exit status information for the most
//                recent process that ended.
//
//    Exit:    None.
//
//    Purpose: Report on the exit status of the most recently zombied process.
//
// *****************************************************************************
//
void myStatus(int pstatus);


// Set up a generic function pointer type so we can collect functions with
// disparate argument lists in one function pointer array. The functions
// will need to be cast to one of the other two types (listed below this
// one) when called into action.
//
typedef void (*generic_fp)();


// A function pointer type that accepts data about the user's command line as
// its arguments. This is used for commands that take command line parameters, 
// like 'cd'.
//
typedef void (*builtin_arg)(char *userArgs[], int numArgs);


// A function pointer type that accepts process status arguments. This 
// is used for commands like 'status'.
//
typedef void (*builtin_arg_proc)(int pstatus);


#endif
