## Jacob Little

## ECE 565 (001)

## Instructor: Dr. Michela Becchi

# Project 2 Report

## Introduction

This report discusses the implementation of Lottery Scheduling and Multi-Level Feedback Queue (MLFQ) Scheduling of user process in the Xinu operating system as well as answers to the questions presented in the project specification.

## Question Answers

**Q1.** The `readylist` is a queue that holds the process that are in the `PR_READY` state. They are ordered by priority (highest priority first) making it easy to compare and remove the highest priority processes first during scheduling. The `readylist` is referenced in `sysinit()` where it is first initialized, in `ready()` where it is used when a process enters the `PR_READY` state, and in `resched()` where it is used for comparing the currently running processes priority against the first item in the `readylist` and also for swapping the currently running process and the first item in the `readylist` when a higher priority process is in the `readylist`.

**Q2.** Xinu uses a priority based round-robin scheduling policy. This policy can lead to process starvation in the event that higher priority processes keep taking up all the runtime and never give a lower priority process the chance to execute because it will always be neglected for higher priority processes.

**Q3.** The `resched` function determines whether the currently running process has a higher priority than the first process in the `readylist` (since the `readylist` is ordered in descending priority, only the first process needs to be checked). If the currently running process has a higher priority, then it will continue to run and not be inserted into the `readylist`. But if the currently running process has a priority that is less that OR equal to the first process in the `readylist`, then it shall be inserted into the `readylist` and hand over execution to the first process in the `readylist`. Note that because the currently running process will change in the case of equal priority with the first item in the `readylist`, round-robin execution is enabled because any number of processes in the readylist with the same priority will be removed and reinserted in a round-robin fashion with each time slice.

**Q4.** All circumstances in which a scheduling event can occur:

- **system/**
  - > **rdssetprio.c** - `rdssetprio()` calls `resched()` after changing the priority of the currently running process.
  - > **clkhandler.c** - `clkhandler()` calls `resched()` once every amount of time set by `QUANTUM` passes for the sake of regular rescheduling.
  - > **kill.c** - `kill()` calls `resched()` when a process kills itself.
  - > **ready.c** - `ready()` calls `resched()` when a process enters the `PR_READY` state.
  - > **receive.c** - `receive()` calls `resched()` when a process blocks for a message.
  - > **recvtime.c** - `recvtime()` calls `resched()` when a process blocks a set amount of time for a message.
  - > **resched.c** - `resched_cntl()` calls `resched()` when all defer requests have been stopped and the most recent defer attempt was successful.
  - > **sleep.c** - `sleepms()` calls `resched()` when a process enters the `PR_SLEEP` state for a set amount of time.
  - > **suspend.c** - `suspend()` calls `resched()` when a process enters the `PR_SUSPEND` state.
  - > **wait.c** - `wait()` calls `resched()` when a process begins waiting on a semaphore.
  - > **yield.c** - `yield()` calls `resched()` when a process yields voluntarily relinquishes the CPU.

## P1. Lottery Scheduling

This problem involved modifying:

- **include/**
    - > **file.h** - text
- **system/**
    - > **file.c** - text

body

## P2. Multi-Level Feedback Queue (MLFQ) Scheduling

This problem involved modifying:

- **include/**
    - > **file.h** - text
- **system/**
    - > **file.c** - text

body