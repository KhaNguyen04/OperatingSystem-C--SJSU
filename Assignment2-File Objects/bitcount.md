-we are going to write a program to count the number of set bits in a file. for example, let's say we have a file with a single line: 

hi

-how many bits does it have? well, the letter h is 0x68 in ASCII, 
which has 2 bits set in 0x6 and 1 bit set in 0x8. the letter i is 0x69 in ASCII, 
so again 2 bits in 0x6 and 2 bits in 0x9. this gives a total of 2+1+2+2=7. wait, the file has a single line. a complete line ends with a new line, \n,
which has an ASCII value of 0xA, which has 2 bits set. thus there are a total of 9 set bits in the file.

-your task is to write a program in C, bitcount, that will take a single argument: a filename. 
it will open that file, read the content of the file, and count the number of set bits. it will output a single decimal number at the end.

-for example, if we run bitcount with hi.txt  Download hi.txt, we get the following:

$ ./bitcount hi.txt 
9

-the program will exit with a successful (0) exit code:

$ echo $?
0

-i've also provided you with rand.dat  Download rand.dat, which is made up of random bytes and a total of 1267076 set bits. you can test with hi.txt  
Download hi.txtand rand.dat  Download rand.dat, but they are not an exhaustive test of your program. the grader will use additional files.
there are two classes of errors you need to handle:
1) if the program is invoked with the wrong number of arguments, 
you should have an exit code of 1 and print the program name (how it was invoked) with the example usage statement:

$ ./bitcount 
USAGE: ./bitcount filename
$ echo $?
1

$/home/bcr33d/bitcount file1.txt file2.txt
USAGE: ./bitcount filename
$ echo $?
1

2) if the file cannot be accessed, you should have an exit code of 2 and use the perror() function and 
pass the file name as the first parameter to print an error message:

$ ./bitcount filethatdoesnotexist.txt
filethatdoesnotexist.txt: No such file or directory
$ echo $?
2

when you implement your program, you must use the open() and read() system calls to read the bytes of the file so take a look at those.

your whole program must be implemented in a single file name bitcount.c. make sure you compile with C (do not write a C++ program!).

