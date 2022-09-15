## Jacob Little

## ECE 561 (001)

## Instructor: Dr. Michela Becchi

# Project 1 Report

Q1. The maximum number of processes accepted by Xinu is 100 by default. This value is defined by `NPROC` in **Configuration**. The value defined in **Configuration** will overwrite the value defined in **conf.h** which will in turn it will be set to 8 by **process.h**.

Q2. An inline definition in **process.h** called `isbadpid()` defines the criteria for an illegal (or "bad") PID. A PID is illegal if it is less than zero, greater than or equal to the maximum number of processes, or if it corresponds to a process table entry that is not currently being used (e.g. its state is `PR_FREE`).

Q3. The default stack size for a process is 65536 bytes which is defined by `INITSTK` in **process.h**.

Q4. 

Q5. The shell process is created in **main.c** during the main process. It is created once first before the main process enters into a loop and continuously waits for the shell process to end so that it can recreate the shell process and start the cycle anew.

Q6. Process tree immediately after initialization:
![Q6 Diagram](/home/xinu/xinu/project1_files/ECE_565_Project_1_Q4_Diagram.png)

Q7. `receive()` stalls the parent process until its child process ends execution.