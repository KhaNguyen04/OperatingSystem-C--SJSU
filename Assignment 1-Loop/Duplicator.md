Remember in school when you did something wrong, like chewed gum in class, you would have to write "I will not chew gum" 100 times. 
Well, we are going to write a program to help with that. You need to write a program that takes a count as a command-line parameter. 
It will then read a line from stdin and write it to stdout that many times. For example: 

$ ./duplicator 3<br/>
i will not chew gum<br/>
i will not chew gum<br/>
i will not chew gum<br/>
i will not chew gum<br/>
this is easy<br/>
this is easy<br/>
this is easy<br/>
this is easy<br/>

For this assignment must be done in C. It will be graded on an Ubuntu (or Kubuntu) 20.04 system,   
so you are highly encouraged to use that to do your development and testing. To read from stdin, you will need to use getline(), 
remember to pass NULL and 0 as the first two arguments so that the right amount of space will be automatically allocated to hold your line. 
When you have finished with the line, remember to free() it. Note that the line includes the newline character, so you will probably want to remove that.

When you program finishes, it will exit with an exit code of 0.

# ERROR HANDLING

If the count is less than or equal to zero, not provided, or not a number, 
print the message "USAGE: duplicator count\ncount must be an integer greater than 0\n" and exit with a code of 1. 
We can assume that a valid count will fit into an "int". 

# EXAMPLE TESTS

./duplicator 1
hello
./duplicator 2
line 1
line 2
line 3
line 4
line 5
./duplicator 3
i will not chew gum
this is easy
./duplicator 135
i will not chew gum
this is easy
./duplicator


