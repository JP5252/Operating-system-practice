## Justin Pearson
## Purpose
This program takes an input of a -i flag followed by a config file which is
parsed and for each filename in the config file, a new child process will be
created for it.

the process creation will be reported like this:

	Starting report, Tue, Oct 1, 2019 12:10:50 PM
	[0] /var/cam/cmpt360/Cprograms/cpu A, started successfully (pid: 10345)
	[1] /var/cam/cmpt360/Cprograms/mem, started successfully (pid: 10348)
	[2] /var/cam/cmpt360/Cprograms/cpu B, started successfully (pid: 10487)
	[3] badprogram /var/cam/somefile.txt, failed to start

then the parent will check every 5 seconds if and of the
processes have completed and report back which processes are still running and
their CPU and memory usage.

Example Report:

	Normal report, Tue, Feb 1, 2023 12:10:55 PM
	[0] Running, cpu usage: 100%, mem usage: 20 MB
	[1] Running, cpu usage: 50%, mem usage: 100 MB
	[2] Running, cpu usage: 75%, mem usage: 15 MB

## Exiting and Terminating
If a process exits during the 5 second interval, it will report as exited at the
next report. 

Example report with exit:

	Normal report, Tue, Feb 1, 2023 12:10:55 PM
	[0] Running, cpu usage: 100%, mem usage: 20 MB
	[1] Exited
	[2] Running, cpu usage: 75%, mem usage: 10 MB

If there is a time limit specified then when that time limit is reached, every
running child process will be terminated.

Example report time limit reached:

	Terminating, Tue, Feb 1, 2023 12:11:10 PM
	[0] Terminated
	[1] Exited
	[2] Terminated
	Exiting (total time: 20 seconds)

If the process receives SIGINT signal then it must handle that signal, report a
termination message and terminate all the child processes before exiting
cleanly.

example report SIGINT received:

	Signal Received - Terminating, Tue, Feb 1, 2023 12:11:10 PM
	[0] Terminated
	[1] Exited
	[2] Terminated
	Exiting (total time: 12 seconds)

## How to run
It can be run by the commands:

    make 
	OR
	make macD
    then run the executable it makes with:
    ./macD -i "configuation file name"
