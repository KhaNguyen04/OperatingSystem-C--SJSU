-in this assignment you will write a program to manage a persistent bag of strings. the bag will be mapped into a 64K file, which you must create if it doesn't already exist. 
(use open() with the 2nd parameter set to O_RDWR | O_CREAT and the 3rd parameter set to S_IWUSR | S_IRUSR. 
you will use the system call ftruncate() to create an empty 64K file. remember to setup the bag header after you create the empty file. 
if you are opening an existing file, do not use O_CREAT and the 3rd parameter is not needed.) the strings can have spaces, 
but no newlines. the bag can also have duplicate strings. to get full points, 
you must maintain the strings in sorted order.
-the file starts with a header:<br/>
struct bigbag_hdr_s {<br/>
    uint32_t magic;<br/>
    // these offsets are from the beginning of the bagfile or 0 if not set<br/>
    uint32_t first_free;<br/>
    uint32_t first_element;<br/>
}<br/>
-the first 4 bytes are a "magic number" that will be 0xC5149 stored as big-endian. (see be32toh() and htobe32() )

all other numbers in the bag file will be stored as little-endian.

the first_free will be the offset from the beginning of the file of the first chunk of free space in a list of free space chunks. if there is no more space in the bag, first_free will be 0.

the first_element will be the offset from the beginning of the file of the first element in the bag. since the bag is sorted this should be the first element alphabetically. (use strcmp() to compare strings for sorting.) if there are no elements in the bag, first_element will be 0.

each free chunk or element offset will start with an entry header:<br/>
struct bigbag_entry_s {<br/>
    uint32_t next;<br/>
    uint8_t entry_magic;<br/>
    uint16_t entry_len; // does not include sizeof bigbag_entry_s<br/>
    char str[0];<br/>
};
-the next element will be the offset from the beginning of the file to the next element in the bag or 0 if there are no more elements.

the entry_magic will be 0xDA for a string or 0xF4 for free space.

entry_len will be the number of bytes after the header that belong to this entry.

if this entry is for a string, str will be the beginning of the string.
#include "bigbag.h  Download bigbag.h" to get those structures. DO NOT CHANGE bigbag.h  Download bigbag.h. you also do not need to submit it. it will be available to your program when we compile it using: gcc -Wall --std=gnu11 -o bigbag bigbag.c

you can use bigbag_dump.c  Download bigbag_dump.cas an example of how to use mmap as well as enable you to debug files you produce. speaking of files, you can also use this sample file to test with: test.dat Download test.dat

your program will memory map the bag file with mmap() to access the file. all file changes will be through memory accesses.

-USAGE STATEMENT:
when you run the program without any arguments or with invalid arguments you should get:<br/>

$ ./bigbag<br/>
USAGE: ./bigbag [-t] filename<br/>
and an exit code of 1.<br/>

PLEASE MAKE SURE the output of your code matches the format of the examples exactly. we are grading with scripts.

if the -t flag is used, the program works as normal, but no changes are actually saved to the file.

-RUNNING THE PROGRAM
when you run the you will enter one character commands on the input. most of the commands with take a string argument after a space. all commands terminate with a linefeed. use the getline function to read in lines of input. pass in a pointer to a null and a size variable of 0 so that getline will allocate memory to receive the string. when you are finished with the string, don't forget to free it! also, be sure to remove the \n at the end of the string before you put it in the bag.

if you enter an invalid command, your program should print a message of the form:<br/>

$ ./bigbag test.dat<br/>
help<br/>
h not used correctly<br/>
possible commands:<br/>
a string_to_add<br/>
d string_to_delete<br/>
c string_to_check<br/>
l<br/>
an a will add a string to the file. duplicates are allowed. strings should be added in sorted order. after the string is added, a message will be printed to indicate success:<br/>

a a walk in the park<br/>
added a walk in the park<br/>
if there is no space to add the string, the message "out of space" will be printed. otherwise, memory should be allocated using first-fit.<br/>

d will delete a string from the file. if there are duplicates, it will delete one of the strings. like add, a success message will print "deleted XXXX"  where XXXX is the string that was deleted. if the string XXXX is not present, the message "no XXXX" will be printed.<br/>

-IMPLEMENTING DELETE IS OPTIONAL AND WORTH 10 BONUS POINTS.<br/>

c will print a line with the word "found" if the string is present or "not found" if the string is not in the bag.<br/>

l will list all the strings in the bag. if the bag is empty, it will print "empty bag". like:<br/>

$ rm test.dat<br/>
bcr33d@ubuntu:~/CLionProjects/bigbag/cmake-build-debug$ ./bigbag test.dat<br/>
l<br/>
empty bag<br/>
here is a longer example run:<br/>

$ rm test.dat <br/>
$ ./bigbag<br/>
USAGE: ./bigbag [-t] filename<br/>
$ ./bigbag test.dat<br/>
?<br/>
? not used correctly<br/>
possible commands:<br/>
a string_to_add<br/>
d string_to_delete<br/>
c string_to_check<br/>
l<br/>
a zoo<br/>
added zoo<br/>
a who says who?<br/>
added who says who?<br/>
a ben<br/>
added ben<br/>
l<br/>
ben<br/>
who says who?<br/>
zoo<br/>
$ ./bigbag test.dat<br/>
a apple<br/>
added apple<br/>
l<br/>
apple<br/>
ben<br/>
who says who?<br/>
zoo<br/>
$ ./bigbag test.dat<br/>
c apple<br/>
found<br/>
c carrot<br/>
not found<br/>
$ ./bigbag -t test.dat<br/>
a a walk in the park<br/>
added a walk in the park<br/>
l<br/>
a walk in the park<br/>
apple<br/>
ben<br/>
who says who?<br/>
zoo<br/>
$ ./bigbag test.dat<br/>
l<br/>
apple<br/>
ben<br/>
who says who?<br/>
zoo<br/>

SUGGESTED STEPS:<br/>
write your program that check for the -t command line option and the filename. make sure the USAGE message and exit codes are correct when command line parameters are not passed correctly.<br/>
open the test.dat  Download test.dattest file using open() and mmap(). bigbag_dump.c  Download bigbag_dump.ccan serve as an example.<br/>
implement the "l" command to list the elements in order.<br/>
implement the "c" command.<br/>
implement an empty bag creation when the file does not exit. you will need to change the flags on open() and mmap() to allow for writes.<br/>
make sure when you try to list an empty bag the correct message is displayed.<br/>
implement "a" to add to the bag.<br/>
use bigbag_dump.c  Download bigbag_dump.cto debug problems with add and to make sure the data structures are correct after each addition.<br/>
implement the -t flag. (it's just a simple change in how you use mmap().<br/>
OPTIONAL FOR EXTRA CREDIT: implement delete. once basic delete is working make sure you coalesce empty entries when you can.<br/>
