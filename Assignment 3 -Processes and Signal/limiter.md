you have been grading the last operating system assignments and you have run into two problems: (okay there are a bunch of there, but these are the ones we are going to address)

sometimes program get into an infinite loop or run too long. this makes testing everything with a script hard because you have to week out the programs that make your script get stuck.
the programs are supposed to loop through the input reading things in chunks, but some people skip doing a loop by doing a read with one outrageously big buffer.
to address these two problems you want to limit the amount of time and memory that a program has to run. you decide to write a limiter program:

$ ./limiter  <br/>
USAGE: ./limiter timelimit_secs memlimit_mb program args... <br/>

this first parameter of limiter will take the number of seconds that a program should be allowed to run. the next parameter is the amount of memory in megabytes (1024 kilobytes) that a program has to run. next parameters will be the program to run followed by any arguments that it might take. (you should not limit the number of arguments.) if any arguments are missing or the first two arguments are not a number, print the usage.

if a program runs more than the time limit, it should be killed with the SIGKILL signal. the exit code of limiter will be the exit code of the program unless it terminates due to a signal then, the the program should the message "killed due to signaldescription" and the exit code will be 100 + the signal number. you can get the signaldescription from a signal number using strsignal(). if the program does not exist, use perror() with the program name to print the error message and exit with 99.

examples
some of these examples use allocator.c  Download allocator.cto allocate chunks of memory. it tests the memory limiting feature of limiter. you can compile it with cc -o allocator allocator.c .

$ ./limiter  <br/>
USAGE: ./limiter timelimit_secs memlimit_mb program args... <br/>
<br/>
$ echo $?    <br/>
1 <br/>
<br/>
$ ./limiter 3 3 sleep 1 <br/>
$ echo $?               <br/>
0 <br/>
<br/>
$ ./limiter 3 3 sleep 4 <br/>
killed due to Killed <br/><br/>
$ echo $? <br/>
109 <br/><br/>
$ ./limiter 3 3 ./allocator 1 <br/>
allocating 1M <br/>
killed due to Segmentation fault <br/><br/>
$ echo $? <br/>
111<br/><br/>

$ ./limiter 3 6 ./allocator 1 <br/>
allocating 1M <br/>
$ echo $?      <br/> 
0 <br/><br/>
$ ./limiter 3 6 ./allocator 1 2 <br/>
allocating 1M <br/>
allocating 2M <br/>
$ echo $? <br/>                      
0 <br/><br/>
$ ./limiter 3 6 ./allocator 1 2 3 <br/>
allocating 1M <br/>
allocating 2M <br/>
allocating 3M <br/>
killed due to Segmentation fault <br/><br/>
$ echo $? <br/>
111<br/>

$ ./limiter 3 1 ./nothere 1   <br/>
./nothere: No such file or directory <br/><br/>
$ echo $? <br/>
99<br/><br/>
there is a weird case where the exec() succeeds but the program fails to start if you don't have enough memory:

$ ./limiter 3 1 ./allocator 1 <br/>
./allocator: error while loading shared libraries: libc.so.6: failed to map segment from shared object <br/><br/>
$ echo $? <br/>
127<br/><br/>
implementation details
to implement limiter, you will need to use fork() to create a child process that will execvp() the given program and arguments. before you do the execvp(), you must use setrlimit() to set the memory limits in the child (you don't want to set the memory limits on the parent). meanwhile, after fork() the parent will set an alarm for the timeout using alarm(). you have to register a signal handler using signal() to catch the signal when the alarm goes off. if the alarm is triggered, you should kill the child process using the kill() system call with the SIGKILL signal. you will then use the wait() system call to wait for the process to complete. you will need to use the W macros, like WEXITSTATUS() to get the exit code. you can get the signal description from a signal number using strsignal().

suggesting steps
write the program that parses the arguments and coverts/validates the number parameters.
add fork() and do the exec() in the child to start the program.
add wait() use the WEXITSTATUS() to get the exit codes.
do setrlimit() before the exec() to limit the amount of memory used. (RLIMIT_AS)
make sure your wait correctly processes programs that fail due to a signal: /limiter 3 3 ./allocator 1
add alarm() and signal() to get the timeouts working
