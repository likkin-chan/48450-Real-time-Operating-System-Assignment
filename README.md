# 48450-Real-time-Operating-System-Assignment
Semaphore and Pipe for real time file reading/writing

Assignment mark: 19.10/20.00

This program created three threads for reading data from one file and write it to another file thru the pipe-line concept.
Specifically, Thread A writes one line of data from the data.txt to the pipe. (note: the given file data.txt has several lines)
              Thread B reads the data from the pipe.
              Thread C reads the line (data) from Thread B, it would detect the data and only write the content region data 
                       into the output file src.txt.

* To compile the program, please ensure that gcc is installed and run the following command:
```
gcc prog_1.c -o prog_1 -lpthread -lrt -Wall
```
* Please see the project specification/description from the Specification folder
