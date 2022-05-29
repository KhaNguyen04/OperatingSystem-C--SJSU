for this problem, we want to print the most popular words from a set of files. it doesn't seem to hard: open a file, read it a word at a time, stick it in a hashtable to keep track of counts, keep track of the max count, at the end print all the words in the table with the max count. it gets a bit harder when we want to do it fast!

for this program we are going to use threads and a thread-safe queue. your program will take a list of files. if a file does not exist, print an error using perror(). your program will create threads that will process each file in parallel. (the in parallel part is important; we want to do this fast!) as each thread reads a word, it will queue the word to one of four counter threads using a thread-safe queue. to decide the queue to use it will use the bottom two bits of the first letter of the word. the counter threads will use their own hashtables to keep track of the count of the words they are tracking.

once the readers have finished, the counter threads will return the count of their most popular word. the main thread will then list all the words with that most popular count.

use fscanf to read the words of a file. "%ms" is a GNU extension to fscanf that will take care of allocating space for words that are read. for example,<br/><br/>

char *ptr;<br/>
fscanf(fh, "%ms", &ptr);<br/><br/>

will read in a word and allocate enough space to hold the word and assign ptr to the allocated space. you will need to make sure that ptr is free()'ed when you are done using it.

you will need to use pthread_create and pthread_join to manage the threads. you will also use condition variables using pthread_mutex_lock/unlock and pthread_cond_wait/signal. make sure all threads are running concurrently: the threads to read the words from the files and the threads to count the popular words.<br/>
PROGRAM SPECIFICATION
your program (pc.c) will take a list of filenames from the program arguments. it will output the most popular words with their counts with one word per line. if there are multiple most popular words, the counts will all be the same. the only other thing that your program will print out is a error (using perror) for each file that could not be opened.

(there is no limit to the number of filenames. you will certainly be tested with more than 5!)
<br/><br/>
EXAMPLE EXECUTION<br/>
$ ./pc 1099words 399words <br/>
wave 4<br/>
$ ./pc 1099words 399words 799words <br/>
undoing 5<br/>
wave 5<br/>
stratagem 5<br/>
$ ./pc /media/sf_Downloads/1099words<br/> 
jam 3<br/>
vii 3<br/>
$  ./pc 1099words nofile 399words 799words  doesnotexist<br/>
nofile: No such file or directory<br/>
doesnotexist: No such file or directory<br/>
undoing 5<br/>
wave 5<br/>
stratagem 5<br/>
$ ./pc<br/>
$<br/>
SUGGESTED STEPS<br/>
write pc.c to simply print the files names passed on the commandline<br/>
write a function that takes a file name, opens the file with fopen, and reads each word from a file and prints it. call the fuction with argv[1] from main.<br/>
write a thread safe queue. instantiate 4 queues and pass them to the reader function from step 2. instead of printing the word, put it in the correct queue.<br/>
write a function that reads from a queue and prints the word.<br/>
create threads for each file to processes with the reader function and filename. create 4 threads calling the printer function. (they should each read from a different queue.)<br/>
implement a hashtable (you can hardcode it to have 256 entries) with a link list of structures that have a strings and count for each entry.<br/>
modify the writer function to use the hashtable to count the times each word occurs in the file. keep track of the maximum count.<br/>
when the reader threads all finish, queue a sentinel value in each queue indicating the readers have all finished.<br/>
make the 4 threads counting the words return their hashtables and maximum counts to the main thread.<br/>
modify the main thread to print all the words (one on each line) with the maximum count.<br/>

