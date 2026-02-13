TCP Connect Port Scanner (C)

A lightweight TCP connect()-based port scanner written in C that focuses on correctness,clarity,and real-world usability.This project probes a target IP across a specified port range using non-blocking sockets and controlled timeouts.Each connection attempt is interpreted using OS-level error behavior and mapped into meaningful TCP states like open, closed, filtered, or unreachable.

Output is then generated in structured CSV format with UTC timestamps,making it suitable for automation,logging,and further analysis pipelines.

#Reason Behind Existence Of This Project:
Most beginner port scanners online are either,too simplistic to be useful, or too unorganized to learn from.
This one sits in the between:
1.understandable
2.technically correct
3.extensible for real security tooling

It’s built as a foundation for future monitoring and exposure-tracking tools, not just a one-off script.


#Core Features:
 1.TCP connect()-based scanning
 2.Non-blocking socket handling
 3.Explicit timeout control using select()
 4.OS error → TCP state classification
 5.Clean CSV output for automation/log ingestion
 6.UTC ISO-8601 timestamps
 7.Defensive, readable implementation


#How it works (high level)
 1. Accepts:
  a. target IP
  b. start port
  c. end port

 For each port:
  1.creation of  a TCP socket
  2.code settings done to non-blocking mode
  3.attempts connection
  4.waits using select()

Based on OS error response:
1.open
2.closed
3.filtered
4.unreachable
5.unknown


#Outputs result in structured format:
timestamp,ip,port,state
2026-02-09T21:45:30Z,192.168.1.1,22,open
2026-02-09T21:45:30Z,192.168.1.1,80,closed


#Build Instructions
Linux / WSL:
gcc port_scanner.c -o scanner

Usage
./scanner <IP> <START_PORT> <END_PORT>

Example:
./scanner 192.168.1.1 1 1024

#Output States Explained
 State	Meaning:
 1.open-->	TCP connection successful
 2.closed-->	Target actively refused connection
 3.filtered-->	No response within timeout
 4.unreachable-->	Host/network unreachable
 5.unknown-->	Unhandled OS error


#Design Decisions
 1.Non-blocking sockets prevent the scan from freezing.
 2.select() provides controlled waiting instead of blind delays.
 3.CSV output makes integration easy with:
   scripts
   SIEM tools
   monitoring dashboards
Focus was reliability and protocol behavior — not speed.


#REMINDER:
 What this is NOT
   Not a vulnerability scanner
   Not an exploit tool
   Not a brute force engine
   Not a “hacking script”
This is exposure detection — the first layer of defensive security visibility.


#Future Improvements:
 Planned evolution of this project:
   Parallel scanning
   Configurable timeouts
   Result storage per scan
   Exposure change tracking
   Service banner logging
   Automation hooks
This scanner is the base layer for a broader internal security tooling stack.


Learnings:
 Low-level socket programming in C
 Non-blocking I/O handling
 OS-level error interpretation
 Network byte order concepts
 Writing tools meant for monitoring, not just execution
 Most importantly: building systems teaches more than reading about them.



Author Intent

This project is part of a practical cybersecurity learning path focused on:

building tools

understanding networks

thinking in monitoring and visibility

shipping real implementations instead of theory-only work
