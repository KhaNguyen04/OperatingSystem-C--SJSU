we are going to enhance the program we wrote for count the bits. we will create a program that counts the set bits in multiple files, and we will use all the CPU cores at our disposal to do it!

our new program, pbitcount, will take a list of files. for each of these files it will start a process to count the set bits in that file. this will allow us to count the set bits in parallel. 

for example, if we run pbitcount with hi.txt  Download hi.txtand rand.dat  Download rand.dattwice, three processes will get created, and we get the following:

$ ./pbitcount hi.txt rand.dat rand.dat <br/>
2534161<br/>
the program will exit with a successful (0) exit code:<br/>

$ echo $?<br/>
0<br/>
there are two classes of errors you need to handle:

1) if the program is invoked with the wrong number of arguments, you should have an exit code of 1 and print the program name (how it was invoked) with the example usage statement:

$ ./pbitcount<br/>
USAGE: ./pbitcount filenames<br/><br/>
$ echo $?<br/>
1<br/>

2) if the file cannot be accessed, you should have an exit code of 2 and use the perror() function to print an error message:

$ ./bitcount hi.txt filethatdoesnotexist.txt anothermissingfile.txt<br/>
filethatdoesnotexist.txt: No such file or directory<br/><br/>
$ echo $?<br/>
2<br/><br/>
when you implement your program, you must use the functions, open(), and read(), so take a look at those. you must also use fork() to create processes for each file and pipe() to communicate between processes. the goal is to start up a process for each file so that you can count each file concurrently (at the same time). if you are not processing the files concurrently, points will be removed for not using fork() correctly.<br/>
Suggested steps to completing the assignment:<br/>
write a simple program to loop through the filenames in argv and print them.<br/>
use fork() to start up a process for each filename in argv and have each process print the filename it is in charge of. make sure the parent uses wait() to avoid ending before the child processes finish.<br/>
use your code from the last assignment to make each process count the bits for the file it is in charge of and print the total for each thread.<br/>
use pipe() to set up a channel between the parent process and the child processes. have the child process write the total to the pipe instead of printing the total. the parent will read the totals from each child and print the grand total after the last child finishes.<br/>
