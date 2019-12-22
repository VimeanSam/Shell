# CSC 415 - Project 3 - My Shell

## Student Name: Vimean Sam

## Student ID: 915819611
## What the program Does
This program remakes the linux shell command line. The program is the functionality of the linux shell which means it accepts linux command and just like the linux terminal. The program accepts linux command such as "ls" and other linux command including a single pipe "|".
## Build Instructions
Type the command "make" in the terminal.
## Run Instructions
After building project, gcc -I -Wall myshell -o fc appeared on the terminal. Then type ./myshell to run the program
## List Extra Credits comepleted (if non attempted leave blank)
1. I completed the extra credit that prints the current working directory to the prompt. I got the home directory by using getenv("HOME") and chdir() system calls. ~/ is shown when the user is in home directory.
##Screenshot
![](/Screenshots/shell_2.jpg)
