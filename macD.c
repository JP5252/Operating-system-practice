#include "macDfunctions.h"

int main(int argc, char *argv[])
{
	int opt;
	char *filename;

	//option handler
	while ((opt = getopt(argc, argv, ":i:")) != -1) {
		switch (opt) {
		//if has -i flag and a following arg
		case 'i':
			filename = optarg;
			break;

		//has a -i flag no following arg
		case ':':
			fprintf(stderr, "Option '%c' needs an argument.\n", optopt);
			return 1;

		//has unknown options
		case '?':
			fprintf(stderr, "Unknown option '%c'\n", optopt);
			return 1;
		}
	}

	//read_file returns an array consisting of each line in the file
	int num_lines = 0;
	char **lines = read_file(filename, &num_lines);

	//start_processes will start the execution of each path given in the file
	int time_limit = -1,
		num_pids = 0;
	pid_t *pids = start_processes(lines, &num_lines, &time_limit, &num_pids);

	monitor_processes(&time_limit, pids, &num_pids);

	//free memory allocated for path names
	if (lines != NULL) {
		for (int i = 0 ; i < num_lines ; i++) {
			//free allocated memory for path name
			free(lines[i]);
		}
		free(lines);
	}
	//free memory allocated to pids
	free(pids);
} //main()
