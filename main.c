//
// *****************************************************************************
// 
// Author:    Erik Ratcliffe
// Date:      May 26, 2015
// Project:   Program 3 - Smallsh
// Filename:  main.c
// Class:     CS 344 (Spring 2015)
//
//
// Overview:
//    Basic shell with three built-in commands and basic signal handling.
//
// Input:
//    Three built-in commands (cd, status, and exit) or any other command 
//    that can be run from a shell command line. The system path is honored.
//
// Output:
//    Normal output from commands, or basic signal messaging for exec'd processes.
//
// *****************************************************************************
//


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "smallsh.h"


int main()
{

    // User input manipulation
    //
    char userInput[MAX_USER_INPUT + 1];  // Holds string entered by the user
    char *userArgs[MAX_ARGS];            // Holds parsed command line args
    char *redirIn = NULL;                // Filename to redirect stdin
    char *redirOut = NULL;               // Filename to redirect stdout
    char *token;                         // String token for parsing command lines
    int  numArgs = 0;                    // Count of parsed command line args

    // Stdin/Stdout manipulation
    //
    int   fdIn;                          // File descriptor to hold new stdin
    int   fdOut;                         // File descriptor to hold new stdout
    int   stdinFd = dup(STDIN_FILENO);   // File descriptor to hold old stdin
    int   stdoutFd = dup(STDOUT_FILENO); // File descriptor to hold old stdout
    pid_t pid;                           // Currently processed PID

    // Background process list 
    // 
    struct Node *head;                   // Front node in background proc list
    int numNodes = 0;                    // Number of background proc in list

    // Other
    //
    char bg = 0;                         // Flag to signal background process
    char cont = 'y';                     // Flag to signal continuing or exiting

    // Array that will hold pointers to our builtin functions. Initially set
    // this up to be a generic_fp type array. Cast all functions saved in the
    // array to be generic_fp as well. When the functions are called, cast
    // them to the correct 'builtin_[no]arg' type, listed earlier.
    // 
    generic_fp builtins[] = { (generic_fp)myCd, (generic_fp)myStatus };


    // The main loop. Continue processing user input and presenting a command
    // prompt until the user runs the built-in "exit" command.
    //
    do 
    {
      // Process zombies if background processes are in our background 
      // process linked list.
      //
      if(numNodes > 0)
      {
          head = clearChildren(head, &numNodes);
      }

      // Flush stdout to get all messaging "out there" that has been buffered.
      // This should help prevent signal messaging and other messaging from 
      // popping up while other processes are running.
      // 
      fflush(stdout);

      // Present the (very basic) command prompt.
      //
      printf(PROMPT);

      // Read a command line from the user.
      //
      fgets(userInput, MAX_USER_INPUT, stdin);

      // If stdin has reached EOF, return control to the TTY. This is part of a
      // solution for endless test script looping.
      //
      if(feof(stdin)) 
      {
        if(!freopen("/dev/tty", "r", stdin)) 
        {
          perror("Error returning control to /dev/tty");
          exit(1);
        }
      }

      // If the command line contains at least a newline, replace the 
      // trailing newline with a null string terminator.
      //
      if(strlen(userInput) >= 1)
      {
          if(userInput[strlen(userInput) - 1] == '\n')
          {
              userInput[strlen(userInput) - 1] = '\0';
          }
      }

      // After removing the trailing newline, if we have anything left, we have
      // a command line to parse. Use strtok() to break the line into a separate
      // array of arguments and strings.
      //
      if(strlen(userInput) > 0)
      {
          // Start by parsing out just the command line w/o redirection or
          // backgrounding.

          // Grab the first token to get started.
          //
          token = strtok(userInput, " ");

          // Loop through all tokens up to the first instance of <, >, or &, and
          // add them to the command line argument array. Increment the argument
          // counter each time another argument or parsable bit is processed.
          //
          numArgs = 0;
          while((token != NULL) && (strchr("<>&", token[0]) == NULL))
          {
              userArgs[numArgs] = token;
              token = strtok(NULL, " ");
              numArgs++;
          }

          // Very important: terminate the array with NULL. This signals the end
          // of command line arguments passed to exec().
          // 
          userArgs[numArgs] = NULL; 

          // At this point, the command line argument array should be set. If
          // there is anything left, it must be redirection or backgrounding, in
          // that order (based on criteria for the project).
          //
          // In a real program, the token following < or > would be checked for
          // validity. Syntax checking is not a requirement for the project so I
          // am leaving it off. 
          //
          while(token != NULL)
          {
              // If stdin is a filesystem-file...
              //
              if(strncmp(token, "<", 1) == 0)
              {
                  // Get the next token and make it the redir input 
                  //
                  token = strtok(NULL, " ");
                  redirIn = token;
              }
              // If stdout is a filesystem-file...
              //
              else if(strncmp(token, ">", 1) == 0)
              {
                  // Get the next token and make it the redir output
                  //
                  token = strtok(NULL, " ");
                  redirOut = token;
              }
              // If the process is being backgrounded...
              //
              else if(strncmp(token, "&", 1) == 0)
              {
                  bg = 1;

                  // If the user did not specify a file to use as
                  // redirected input, we have to set /dev/null as the
                  // redirected input.
                  //
                  if(redirIn == NULL)
                  {
                      redirIn = "/dev/null";
                  }
              }
              token = strtok(NULL, " ");
          }

          // Compare the first index of the command arguments array against
          // two of the three built-in commands (cd and status). The last of 
          // the built-ins, exit, is not tied to any function but instead just
          // changes a flag that signals the end of the do/while loop we're in now.
          //
          // Also check to see if the command line begins with a #. If so, ignore
          // the line (it's a comment).
          //
          if(strncmp(userArgs[0], "#", 1) != 0)                  // comment
          {
              if(strncmp(userArgs[0], "cd", 2) == 0)             // "cd"
              {
                  ((builtin_arg)builtins[0])(userArgs, numArgs); // run myCd() 
              } 
              else if(strncmp(userArgs[0], "status", 6) == 0)    // "status"
              {
                  ((builtin_arg_proc)builtins[1])(pstatus);      // run myStatus()
              }
              else if(strncmp(userArgs[0], "exit", 4) == 0)      // "exit"
              {
                  // Set the continuation flag to 'n' so the 
                  // shell can exit.
                  //
                  cont = 'n';
              }
              else
              {
                  // Fork this shell. The fork() function will return -1 if an
                  // error was encountered, or 0 if the currently running process
                  // is the one the parent forked, or the PID of the fork()'d
                  // child process if the current process is the child's parent.
                  // 
                  pid = fork(); 

                  // If anything went awry when forking this shell, exit with
                  // a descriptive error.
                  //
                  if((int)pid < 0)
                  {
                      perror("Fork failed.");
                      exit(1);
                  }
                  else if((int)pid == 0) 
                  {
                      // This section of the code will only be seen by the fork()'d
                      // child process.

                      // SIGINT is ignored in the parent process. Now that we're in
                      // the child process, we need to set it back to normal (SIG_DFL).
                      //
                      struct sigaction saInt;
                      saInt.sa_handler = SIG_DFL;
                      saInt.sa_flags = 0;

                      // If there was an error setting up the sigaction(), exit with
                      // a descriptive error.
                      //
                      if(sigaction(SIGINT, &saInt, 0) == -1)
                      {
                          perror("SIGINT ignore sigaction failed");
                          exit(1);
                      }

                      // If the command was put in the background (this was set when
                      // user input was parsed and a "&" was detected at the end of the
                      // command line), report the background PID.
                      //
                      if(bg == 1)
                      {
                          printf("background pid is %d\n", (int)getpid());
                          fflush(stdout);
                      }

                      // If the user entered a file for stdin redirection, reassign
                      // stdin to that file.
                      //
                      if(redirIn != NULL)
                      {
                          // Open the new stdin file as read-only.
                          //
                          fdIn = open(redirIn, O_RDONLY);

                          // If an error was encountered, exit with a descriptive
                          // error message.
                          //
                          if(fdIn < 0)
                          {
                              perror("Failed to open file for redirected input");
                              exit(1);
                          }

                          // Duplicate stdin on the new stdin file descriptor. If an
                          // error is encountered, exit with a descriptive error message.
                          //
                          if(dup2(fdIn, STDIN_FILENO) == -1)
                          {
                              perror("Stdin dup2()");
                              exit(1);
                          }
                          
                          // It seems counterintuitive, but now that we have
                          // duplcated the stdin file descriptor to a different
                          // file descriptor, we can close out the new stdin
                          // descriptor file. It's not needed anymore. 
                          //
                          close(fdIn);
                          
                          // Reset the stdin redirection file string so it can be used 
                          // again with a minimum of fuss.
                          //
                          redirIn = NULL;
                      }

                      // If the user entered a file for stdout redirection, reassign
                      // stdout to that file.
                      //
                      if(redirOut != NULL)
                      {
                          // Open the new stdout file. Set permissions to read/write by 
                          // the user, and set up the file to be write-only from this 
                          // program. If the file does not exist, create it; if it does 
                          // exist, truncate it.
                          //
                          fdOut = open(redirOut, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                          fcntl(fdOut, FD_CLOEXEC, 1);

                          // If an error was encountered, exit with a descriptive
                          // error message.
                          //
                          if(fdOut < 0)
                          {
                              perror("Failed to open file for redirected output");
                              exit(1);
                          }

                          // Unlike with stdin, there is no controversy whatsoever with using
                          // fflush() with stdout. Similar to stdin, it's best to flush
                          // out stdout before shifting it to a different file descriptor.
                          //
                          fflush(stdout);
                          if(dup2(fdOut, STDOUT_FILENO) == -1)
                          {
                              perror("Stdout dup2()");
                              exit(1);
                          }

                          // Now that we have duplcated the stdin file
                          // descriptor to a different file descriptor, we can
                          // close out the new stdin descriptor file. It's not
                          // needed anymore. 
                          //
                          close(fdOut);
                      }

                      // Exec the command line entered by the user. This will replace
                      // the existing fork()'d process with the new command's process.
                      //
                      execvp(userArgs[0], userArgs);

                      // If everything goes well, we will never get here. A successful 
                      // exec() ends the program here and never returns.
                      //
                      // If an error occurred, as always exit with a descriptive message.
                      //
                      perror("Exec failed");
                      exit(1);
                  }
                  else 
                  {
                      // This section of the code will only be seen by the parent
                      // of the fork()'d child process.

                      // SIGINT is ignored in the parent process (SIG_IGN). We do not
                      // want SIGINT to be ignored in child processes, though, so 
                      // similar code in the child process section above changes this
                      // signal handling back to its defaults (SIG_DFL).
                      //
                      struct sigaction saInt;
                      saInt.sa_handler = SIG_IGN;
                      saInt.sa_flags = 0;

                      // If there was an error setting up the sigaction(), exit with
                      // a descriptive error.
                      //
                      if(sigaction(SIGINT, &saInt, 0) == -1)
                      {
                          perror("SIGINT ignore sigaction failed");
                          exit(1);
                      }

                      // If we backgrounded the process, track it in the process 
                      // linked list.
                      //
                      if(bg == 1)
                      {
                          // If we have no populated nodes, populate head. Otherwise,
                          // add a new node to the list. The fork() process returned
                          // the child's PID to us (it's what got us here), so plug 
                          // that into a node in the linked list manually. This only 
                          // happens this way for the first node; subsequent nodes are 
                          // added via addNode().
                          //
                          if(numNodes == 0) {
                              // Set up the initial head node
                              //
                              head = (struct Node *) malloc(sizeof(struct Node));
                              head->next = NULL;
                              head->pid = pid;
                              numNodes++;
                          } else {
                              // Otherwise, add a new node to the linked list.
                              head = addNode(pid, head);
                              numNodes++;
                          }
                      }
                      else{
                          // We did not spawn a background process (those are all handled
                          // separately through the linked list that was started above),
                          // so block until the current FOREground process ends. Write its
                          // exit status data to the global pstatus variable so other
                          // functions can glean information from it.
                          //
                          waitpid(pid, &pstatus, 0);
                      }
                  }

                  // Reset our input file, output file, background flag, and user 
                  // input string. 
                  //
                  redirIn = NULL;
                  redirOut = NULL;
                  bg = 0;
                  userInput[0] = '\0';
              }
          }
      }

    } while(cont == 'y');

    // At this point, the user has entered "exit" to leave the shell. Restore 
    // stdin/stdout to their normal settings
    
    // Reset stdin to its original file descriptor status.
    //
    if(dup2(stdinFd, STDIN_FILENO) == -1)
    {
        perror("Stdin restoration with dup2()");
        exit(1);
    }

    // Close the temporary stdin file descriptor. We don't need it anymore.
    //
    if(close(stdinFd) == -1)
    {
        perror("Close stdinFd");
        exit(1);
    }
        
    // As before, some level of flushing is always good when changing stdout to 
    // something else.
    //
    fflush(stdout);

    // Reset stdout to its original file descriptor status.
    //
    if(dup2(stdoutFd, STDOUT_FILENO) == -1)
    {
        perror("Stdout restoration with dup2()");
        exit(1);
    }

    // Close the temporary stdout file descriptor. We don't need it anymore.
    //
    if(close(stdoutFd) == -1)
    {
        perror("Close stdoutFd");
        exit(1);
    }

    exit(EXIT_SUCCESS);

}

