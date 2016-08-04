//
// *****************************************************************************
// 
// Author:    Erik Ratcliffe
// Date:      May 26, 2015
// Project:   Program 3 - Smallsh
// Filename:  smallsh_func.c
// Class:     CS 344 (Spring 2015)
//
//
// Overview:
//    Basic shell with three built-in commands and basic signal handling.
//
//    This file contains supplementary functions called upon by main().
//
// *****************************************************************************
//


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "smallsh.h"


// *****************************************************************************
// 
// void myCd(char *userArgs[], int numArgs)
//
// Purpose: Built-in cd command, light imitation of UNIX cd.
//
// *****************************************************************************
//
void myCd(char *userArgs[], int numArgs)
{

    char *homeDir;    // Holds the user's home directory

    // If there's 1 arg on the command line, the user's home directory is requested.
    // If there are two args, the user specified a directory to go to.
    // If more than two args, the user is not using cd correctly.
    //
    switch(numArgs)
    {
        case 1:
            homeDir = getenv("HOME");  // grab the user's home dir envar
            chdir(homeDir);            // change to the directory
            break;
        case 2:
            chdir(userArgs[1]);        // 2nd arg is the directory to go to
            break;                     // change to the directory
        default:
            printf("Invalid: too many arguments to cd.\n");
    }

}


// *****************************************************************************
// 
// void myStatus(int pstatus)
//
// Purpose: Reports exit/termination status of the most recent zombied process.
//
// *****************************************************************************
//
void myStatus(int pstatus)
{
    if(WIFEXITED(pstatus))                                       // normal exit?
    {
        printf("exit value %d\n", WEXITSTATUS(pstatus));         // exit status
    }
    else if(WIFSIGNALED(pstatus) != 0)                           // terminated?
    {
        printf("terminated by signal %d\n", WTERMSIG(pstatus));  // term signal
    }
    else if(WIFSTOPPED(pstatus) != 0)                            // stopped by OS?
    {
        printf("stopped by signal %d\n", WSTOPSIG(pstatus));     // stopped signal
    }
    else if(WIFCONTINUED(pstatus) != 0)                          // continued?
    {
        printf("process continued\n");  
    }
}


// *****************************************************************************
// 
// struct Node *addNode(int newPid, struct Node *head)
//
// Purpose: Adds a background process PID node to the linked list that tracks
//          background processes.
//
// *****************************************************************************
//

struct Node *addNode(int newPid, struct Node *head) {

    // Set up a new node to add to the background PID node list.
    // 
    struct Node *newNode;      
    newNode = (struct Node *) malloc(sizeof(struct Node));
    newNode->pid = newPid;

    // Add the new node to the head of the list for simplicity. (It's 
    // always harder adding nodes to the end of the list.)
    //
    newNode->next = head;

    // Return the new node pointer. This value is used by the calling
    // function to let it know the new head of the list.
    //
    return newNode;

}


// *****************************************************************************
// 
// struct Node *clearChildren(struct Node *head, int *numNodes)
//
// Purpose: Reaps zombies from background processes. Removes nodes of reaped
//          zombies afterward.
//
// *****************************************************************************
//
struct Node *clearChildren(struct Node *head, int *numNodes) {

    int wpid;                 // PID returned by waitpid()
    struct Node *prev;        // Previous node in the linked list
    struct Node *curr;        // Current node in the linked list

    prev = NULL;              // Start off at the beginning (no previous node)
    curr = head;              // Start at the head node

    // Only move head if it's being freed! Otherwise, maintain head.
    // (Head = no prev node.)
    
    // For each node in the list...
    //
    while(curr != NULL)
    {
        // Try to reap the zombie from the process. Assign the return value
        // to wpid and use that to determine how to move forward. Do not wait
        // for a zombie to be reaped; try it and move along (WNOHANG).
        //
        // Upon success, update the global pstatus variable to contain exit
        // data for the process.
        //
        wpid = (int)waitpid(curr->pid, &pstatus, WNOHANG);

        // If we succeeded in reaping a zombie...
        //
        if(wpid > 0)
        {
            // Report which PID was reaped.
            //
            printf("background pid %d is done: ", wpid);

            // Check the pstatus variable for exit/termination information
            // and report it.
            //
            myStatus(pstatus);

            // Head is being freed! Move head to head->next and free curr. The
            // PID that was in curr will not be checked again.
            //
            if(prev == NULL)
            {
                head = head->next;
                free(curr);
                curr = head;
            }
            // Head is not being freed. Point prev->next to curr->next and
            // free curr.
            //
            else{
                prev->next = curr->next;
                free(curr);
                curr = prev->next;
            }

            // One less PID to process. Keep count.
            //
            (*numNodes)--;
        }
        // ...otherwise, just move to the next node in the list.
        //
        else   
        {
            prev = curr;
            curr = curr->next;
        }
    }

    // Return the head pointer in case it is needed.
    //
    return head;
}



