#include "macDfunctions.h"

int stop;

char **read_file(const char *filename, int *num_lines)
{
	FILE *fp;
    //initialize lines with space for 10
	char **lines = malloc(10 * sizeof(char *));
	char line[PATH_MAX];

    //open file for reading
	fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Error opening file %s\n", filename);
		free(lines);
		return NULL;
	}

	//read file line by line
	while (fgets(line, PATH_MAX, fp)) {
		//remove trailing newline character
		if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = '\0';

		//allocate memory for the line string
		if (*num_lines >= 10) {
			lines = realloc(lines, (*num_lines + 1) * sizeof(char *));
			if (lines == NULL) {
				fclose(fp);
				return NULL;
			}
		}

		//allocate memory for line then copy it
		lines[*num_lines] = malloc(strlen(line) + 1);
		if (lines[*num_lines] == NULL) {
			fclose(fp);
			return NULL;
		}
		strncpy(lines[*num_lines], line, strlen(line) + 1);

		(*num_lines)++;
	}
	//close file and return lines
	fclose(fp);
	return lines;
} //read_file()

pid_t *start_processes(char **path_names, int *name_count,
					   int *time_limit, int *pid_count)
{
	//initialize pids with space for 10
	pid_t *pids = malloc(10 * sizeof(pid_t));
	pid_t pid;
	int count = 0;
	//this is incremented if a time limit is defined so output is correct
	int is_limit = 0;

	/*For each path in path_names:
	 *	- Determine if the path is to a valid executable
	 *	- Start executable if it is
	 */
	if (path_names != NULL) {
		char *path_name = malloc(PATH_MAX);
		char *currentTime = get_time();

		printf("Starting report, %s\n", currentTime);
		free(currentTime);

		for (int i = 0 ; i < *name_count ; i++) {
			strncpy(path_name, path_names[i], PATH_MAX-1);

			//check for timer definition line
			if (strstr(path_name, "timelimit")) {
				char *token = strtok(path_names[i], " ");

				token = strtok(NULL, " ");
				*time_limit = atoi(token);
				is_limit++;
				continue;
			}
			//run path in start_exec and record the pid
			else {
				pid = start_exec(path_names[i]);

				if (count >= 10)
					pids = realloc(pids, (count + 1) * sizeof(pid_t *));

				//if process starts successfully
				if (pid > 0) {
					pids[count] = pid;
					printf("[%d] %s, started successfully (pid: %d)\n",
					i - is_limit, path_name, pid);
					*pid_count = *pid_count + 1;
				}
				//if process fails to start
				else
					printf("[%d] %s, failed to start\n", i - is_limit, path_name);
				count++;
			}
		}
		free(path_name);
	}
	//change name_count to only include processes that started adn return
	return pids;
} //start_processes()

pid_t start_exec(char *path_name)
{
	char *args[15];
	int count = 1;
	//set up for execvp
	//check if there are any spaces in the line
	char *token = strtok(path_name, " ");

	//check if the path is an executable, return 0 if not
	if (!access(token, X_OK) == 0)
		return 0;

	//if spaces are present we split up path into array with arguments
	if (!strcmp(token, path_name)) {
		args[0] = token;
		while ((token = strtok(NULL, " ")) != NULL)
			args[count++] = token;
		//set lest element to NULL
		args[count] = NULL;
	}

	//if there are no spaces present, just plug path name directly in
	else {
		args[0] = path_name;
		args[1] = NULL;
	}

	//fork child process and record pid
	pid_t pid = fork();

	//check if fork failed
	if (pid < 0)
		return pid;

	//pid == 0 means we are in the child process
	else if (pid == 0) {
		//put child in its own process group so SIGINT does not kill
		setpgid(0, 0);
		//redirect output so that it is not displayed
		freopen("/dev/null", "w", stdout);
		freopen("/dev/null", "w", stderr);
		//run execvp to replace child process with given process
		execvp(args[0], args);
		exit(1);
	}

	else
		return pid;
} //start_exec()

void monitor_processes(int *time_limit, pid_t *pids, int *num_pids)
{
	time_t start_time = time(NULL);
	int time_elapsed = (time(NULL) - start_time);
	struct sigaction sa;

	//set timer to 999 if timer is not specified
	if (*time_limit == -1)
		*time_limit = 999;

	memset(&sa, 0, sizeof(sa));	// zero out the whole structure
	sa.sa_flags = 0;			// for the "simpler" handler, this is 0
	sa.sa_handler = handler;	// assign our handler's address

	//register the signal handler to be called when SIGINT is received.
	if (sigaction(SIGINT, &sa, NULL) == -1)
		err(1, "sigaction");

	//runs until time is greater than time, sigint received or no child processes
	while (time_elapsed <= *time_limit && !stop && *num_pids != 0) {
		//sleep for 5 seconds and mark elapsed_time
		sleep(5);
		time_elapsed = (time(NULL) - start_time);

		//if SIGINT received break out of loop
		if (stop)
			break;

		char *currentTime = get_time();

		printf("\nNormal report, %s\n", currentTime);
		free(currentTime);

		//check current status of process
		for (int i = 0; i < *num_pids; i++) {
			pid_t pid = pids[i];
			int status;
			//if process is still active
			if (waitpid(pid, &status, WNOHANG) == 0) {
				float cpu_usage = get_cpu_usage(pid);
				int mem_usage = get_mem_usage(pid);

				printf("[%d] Running, cpu usage: %.f%%, mem usage: %d MB\n",
				i, cpu_usage, mem_usage);
			}

			//if process is no longer active
			else if (pid == -1 || !waitpid(pid, &status, WNOHANG) == 0) {
				printf("[%d] Exited\n", i);
				pid = -1;
			}
		}

		//if all processes have terminated, break out of the loop
		int all_terminated = 1;

		for (int i = 0; i < *num_pids; i++) {
			if (pids[i] != -1) {
				all_terminated = 0;
				break;
			}
		}

		if (all_terminated) {
			printf("\n");
			break;
		}

		//if there is less than 5 seconds left only wait the difference
		if ((*time_limit - time_elapsed) < 5) {
			//sleep the difference
			sleep((*time_limit - time_elapsed));
			time_elapsed = (time(NULL) - start_time);
			//if SIGINT received break out of loop
			if (stop)
				break;
			break;
		}
	}

	char *currentTime = get_time();

	printf("Terminating, %s\n", currentTime);
	free(currentTime);
	//kill processes still running
	for (int i = 0; i < *num_pids; i++) {
		pid_t pid = pids[i];
		int status;
		//if process is still active
		if (waitpid(pid, &status, WNOHANG) == 0) {
			printf("[%d] Terminated\n", i);
			kill(pid, 1);
			pid = -1;
		}

		//if process exited in loop
		else if (pid == -1 || !waitpid(pid, &status, WNOHANG) == 0) {
			printf("[%d] Exited\n", i);
			pid = -1;
		}
	}
	printf("Exiting (total time: %d seconds)\n", time_elapsed);
} // monitor_processes()

char *get_time(void)
{
	time_t currentTime;
	struct tm *localTime;
	char *timeString;

	//get the current time
	currentTime = time(NULL);

    //convert to local time
	localTime = localtime(&currentTime);

    //allocate memory for the timeString
	timeString = (char *)malloc(sizeof(char) * 30);

	//format the timeString using strftime()
	strftime(timeString, 30, "%a, %b %d, %Y %I:%M:%S %p", localTime);

	//return timeString
	return timeString;
} //get_Time()

void handler(int sig)
{
	if (sig == SIGINT) {
		printf("\nSignal Received - ");
		stop = 1;
	}
} //handler()

float get_cpu_usage(pid_t pid)
{
	char *line = malloc(CHAR_MAX),
		 *filename = malloc(PATH_MAX);
	FILE *file;
	unsigned long utime, stime, starttime, uptime_sec;
	long clock_ticks;
	float elapsed_sec, usage_sec, cpu_usage;

	//open file with stat which has our cpu usage times
	sprintf(filename, "/proc/%d/stat", pid);
	file = fopen(filename, "r");
	if (file == NULL)
		return 0;

	if (fgets(line, CHAR_MAX, file) == NULL)
		return 0;

	fclose(file);
	//get utime and stime to determine cpu runtime and starttime to determine uptime
	int check = sscanf(line, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %lu", &utime, &stime, &starttime);

	//check return of sscanf()
	if (check != 3)
		return 0;

	//open up uptime to get the total cpu uptime
	file = fopen("/proc/uptime", "r");
	if (file == NULL)
		return 0;

	if (fscanf(file, "%lu", &uptime_sec) != 1)
		return 0;

	fclose(file);

	//set clock_ticks to system clock ticks/sec
	clock_ticks = sysconf(_SC_CLK_TCK);

	//get an elapsed time of process run in seconds
	elapsed_sec = (float) (uptime_sec - (starttime / clock_ticks));

	//get a usage time in seconds
	usage_sec = (float) (utime + stime) / clock_ticks;

	//get the cpu usage by dividing usage time by run time * 100
	cpu_usage = usage_sec / elapsed_sec * 100;

	//free allocated memory
	free(line);
	free(filename);

	return cpu_usage;
} //get_cpu_usage()

int get_mem_usage(pid_t pid)
{
	FILE *fp;
	char *filename = malloc(PATH_MAX),
		 *line = malloc(CHAR_MAX);
	long size;
	float memory_usage;

	//open up statm to read out memory stats
	sprintf(filename, "/proc/%d/statm", pid);
	fp = fopen(filename, "r");
	if (fp == NULL)
		return 0;

	//read first line and close the file
	fgets(line, sizeof(line), fp);
	fclose(fp);

	//get the size from line using strtol() to make checkpatch happy
	char *endptr;

	size = strtol(line, &endptr, 10);

	//calculate memory usage by multiply size by page size in bytes then divide to
	//convert from bytes to MB
	memory_usage = ((int)size * sysconf(_SC_PAGESIZE)) / (1024.0 * 1024.0);

	free(filename);
	free(line);

	return memory_usage;
} //get_mem_usage()
