# EVIDENCE – What to Capture and Submit

## Purpose of This Document

This document explains **what evidence you are expected to capture and submit** as you build the
real-time embedded system throughout the course.

Real-time systems are evaluated based on **observed behavior**, not intended behavior.
Your submissions should demonstrate that the system behaves correctly under real-time constraints.

This document is referenced throughout the guided labs and should be consulted every module.

---

## General Evidence Expectations (All Modules)

For each guided lab and project milestone, you should capture:

- Console output or logs showing system behavior
- Screenshots or recordings demonstrating execution
- Brief written notes explaining what the evidence shows

Evidence should clearly support claims about:

- Concurrency
- Timing behavior
- Synchronization correctness
- Communication flow
- Event-driven response
- System stability

---

## Module-by-Module Evidence Checklist

### Module 2 – Project Initialization

Capture evidence showing:

- Successful build of the project
- Successful run of the baseline system
- Expected initial output or “heartbeat” behavior
- Confirmation that the project runs without errors

Suggested evidence:
- Screenshot of build output
- Screenshot or log of program execution
<img width="1434" height="525" alt="image" src="https://github.com/user-attachments/assets/c55a0f90-9534-4765-98d6-4eb0923da4c7" />

---

### Module 3 – Concurrency and Task Control

Capture evidence showing:

- Two or more RTOS tasks executing concurrently
- Interleaved or coordinated task output
- Evidence that tasks are scheduled independently

Suggested evidence:
- Console output showing interleaved task messages
- Brief explanation of how concurrency is observed

---

### Module 4 – Timing, Timers, and Watchdogs

Capture evidence showing:

- Periodic execution driven by timers
- Timing measurements (timestamps, intervals, counters)
- Watchdog-style detection of delays or stalled execution

Suggested evidence:
- Output showing periodic timing behavior
- Output showing watchdog activation under a fault condition
- Notes explaining normal vs. abnormal behavior

---

### Module 5 – Synchronization and Resource Sharing

Capture evidence showing:

- Shared resource access protected by synchronization mechanisms
- Correct behavior under concurrent access
- Absence of race conditions or corrupted output

Suggested evidence:
- Output demonstrating stable shared-state behavior
- Optional before/after comparison (without vs. with protection)
- Notes describing why the chosen synchronization mechanism was used

---

### Module 6 – Inter-Task Communication

Capture evidence showing:

- Messages being sent and received between tasks
- Structured and predictable message flow
- Handling of blocking or timeout behavior

Suggested evidence:
- Output showing producer/consumer message exchange
- Notes explaining queue size and blocking decisions


<img width="964" height="520" alt="image" src="https://github.com/user-attachments/assets/07091ffd-1939-41cc-9ad0-c34f6bc9c473" />

- Added IPC requirements that track communication between tasks and handle message queues and shared resource handling.
---

### Module 7 – Interrupts, Signals, and Event-Driven Behavior

Capture evidence showing:

- An asynchronous event triggering system behavior
- Minimal, bounded work in interrupt context
- Transfer of work from ISR/event to task context

Suggested evidence:
- Output showing event trigger and task-side handling
- Notes explaining ISR discipline and responsiveness impact

---

### Module 8 – System Integration and Stabilization

Capture evidence showing:

- Integrated system behavior across all components
- Identification of responsiveness or reliability issues
- Improvements made and their impact on system behavior

Suggested evidence:
- Logs showing behavior before and after changes
- Notes describing what was changed and why it improved the system

---

### Module 9 – Final Project Submission

Your final submission should include evidence demonstrating:

- Concurrent task execution
- Correct timing behavior
- Reliable synchronization and IPC
- Event-driven response
- Stable system operation under expected conditions

Suggested evidence:
- Demonstration recording or live demo
- Consolidated logs/screenshots
- Written analysis referencing observed behavior

---

## Quality Guidelines

- Evidence should be **clear and readable**
- Output should be **annotated or explained**
- Avoid excessive or irrelevant logs
- Focus on behavior that supports learning outcomes

---

## Final Reminder

The strongest submissions clearly connect:

- **What you implemented**
- **What you observed**
- **Why the system behaves that way**

Use evidence to support your conclusions and design decisions.
