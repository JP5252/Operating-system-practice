/********************************************************************************
 * This is the header file for macDfunctions.c, it includes 8 functions that are
 * used in the implementation of macD.c.
 * The functions in this header file are:
 *  - read_file()
 *  - start_processes()
 *  - start_exec()
 *  - moniter_processes()
 *  - get_time()
 *  - handler()
 *  - get_cpu_usage()
 *  - get_mem_usage()
**********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <time.h>
#include <sys/resource.h>
#include <err.h>

//global to be used by signal handler to indicate when SIGINT is receieved
extern int stop;

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

/* This is a helper function for macD.c that opens a given file if it exists and
 *  splits up the file into lines for processing by start_processes().
 * This receives 2 parameters:
 *  - filename which is a pointer to a filename string.
 *  - num_lines which is a pointer to an integer that is set to 0 and will keep
 *    count of the amount of lines in the file.
 * This returns a dynamically allocated array of pointers to each line stored
 *  as a string.
 */
char **read_file(const char *filename, int *num_lines);

/* This is a helper function for macD.c that starts all the processes from the
 *  given configuration file.
 * This receives 4 parameters:
 *  - path_names which is an array of pointers to strings to parse through.
 *  - name_count which is the amount of items in the path_names array.
 *  - time_limit which will be set if there is a time limit specified in the
 *    first line of the path_names array
 *  - pid_count which is a pointer to an int which will be incrememented every
 *    time a child process is created
 * This returns a dynamically allocated list of child process ID's in pid_t.
 */
pid_t *start_processes(char **path_names, int *name_count,
					   int *time_limit, int *pid_count);

/* This is a helper function for start_processes that takes a path name and
 *  first checks if its an executable, if it is it then forks a child process and
 *  uses execvp to run the executable in that child process and returns the process
 *  ID of the child process.
 * This receives 1 parameter:
 *  - path_name which is a pointer to a path name string.
 * This returns a process ID of type pid_t of the child process created if
 *  successeful and 0 if unsuccessful.
 */
pid_t start_exec(char *path_name);

/* This is a helper function for macD.c that takes a list of process ids and a
 *  time limit and runs the started child processes until the time limit is reached
 *  and terminates them properly.
 * This receives 3 parameters:
 *  - time_limit which is a pointer to an integer this determines how long the
 *    child processes will be monitered for. If no time limit was specified in
 *    the configuration files the time_limit will be set arbitrarily high to 999.
 *  - pids which is a list of process ID's of the child processes we are going
 *    to moniter.
 *  - num_pids which is a pointer to an integer of the amount of child processes
 *    in pids.
 * This returns nothing.
 */
void monitor_processes(int *time_limit, pid_t *pids, int *num_pids);

/* This is a helper function for macD.c that takes no input and returns the
 *  current date and time in a formatted string when called.
 * This a string formatted like:
 *  "Wed, Mar 08, 2023 10:40:36 PM"
 */
char *get_time(void);

/* This is a signal handler for macD.c that is only called in
 *  moniter_processes() so that the function can kill all the processes it
 *  creates and give some output for the program quits.
 * This receives 1 parameter:
 *  - sig which is a signal Id.
 * This returns prints a string "Signal Received - " and sets the global stop
 *  to 1 to let the while loop in moniter_processes() know to break its loop
 *  and terminate all the processes when it receives the SIGINT signal.
 */
void handler(int sig);

/* This is a helper function for moniter_procesess that opens proc/PID/stat and
 *  proc/uptime to grab information and determine the CPU usage by percentage.
 * This receives 1 parameter:
 *  - pid which is the process ID of the process we want the CPU% of.
 * This returns a float value of the cpu usage in percent.
 */
float get_cpu_usage(pid_t pid);

/* This is a helper function for moniter_procesess that opens proc/PID/statm to
 *  grab information on the memory usage of the program and then converts it to MB.
 * This receives 1 parameter:
 *  - pid which is the process ID of the process we want the memory usage of.
 * This returns an integer value of the memory usage in MB.
 */
int get_mem_usage(pid_t pid);

#endif
