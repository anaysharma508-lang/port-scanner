# TCP Connect Port Scanner (C)

A lightweight TCP `connect()`-based port scanner written in C that focuses on correctness, clarity, and real-world usability.  
This project probes a target IP across a specified port range using non-blocking sockets and controlled timeouts. Each connection attempt is interpreted using OS-level error behavior and mapped into meaningful TCP states like **open**, **closed**, **filtered**, or **unreachable**.

Output is generated in structured **CSV format with UTC timestamps**, making it suitable for automation, logging, and further analysis pipelines.

---

## Reason Behind Existence Of This Project

Most beginner port scanners online are either:

- Too simplistic to be useful  
- Too unorganized to learn from  

This project sits in between:

1. Understandable  
2. Technically correct  
3. Extensible for real security tooling  

It is built as a foundation for future monitoring and exposure-tracking tools, not just a one-off script.

---

## Core Features

1. TCP `connect()`-based scanning  
2. Non-blocking socket handling  
3. Explicit timeout control using `select()`  
4. OS error → TCP state classification  
5. Clean CSV output for automation / log ingestion  
6. UTC ISO-8601 timestamps  
7. Defensive, readable implementation  

---

## How It Works (High Level)

**Inputs:**

- Target IP  
- Start port  
- End port  

**Per Port Workflow:**

1. Creation of a TCP socket  
2. Socket switched to non-blocking mode  
3. Connection attempt via `connect()`  
4. Wait using `select()` with explicit timeout  
5. Interpretation of OS-level error codes  

**Resulting States:**

- Open  
- Closed  
- Filtered  
- Unreachable  
- Unknown  

---

## Output Format

Structured CSV:

```
timestamp,ip,port,state
2026-02-09T21:45:30Z,192.168.1.1,22,open
2026-02-09T21:45:30Z,192.168.1.1,80,closed
```

---

## Build Instructions (Linux / WSL)

```bash
gcc port_scanner.c -o scanner
```

---

## Usage

```bash
./scanner <IP> <START_PORT> <END_PORT>
```

**Example:**

```bash
./scanner 192.168.1.1 1 1024
```

---

## Output States Explained

| State        | Meaning |
|-------------|----------|
| **open**        | TCP connection successful |
| **closed**      | Target actively refused connection |
| **filtered**    | No response within timeout |
| **unreachable** | Host / network unreachable |
| **unknown**     | Unhandled OS error |

---

## Design Decisions

1. Non-blocking sockets prevent scan freezes  
2. `select()` enables controlled waiting instead of blind delays  
3. OS error codes drive state classification  
4. CSV output simplifies integration with:

   - Scripts  
   - SIEM tools  
   - Monitoring dashboards  

Focus was reliability and protocol behavior — **not scan speed**.

---

## Reminder: What This Is NOT

- Not a vulnerability scanner  
- Not an exploit tool  
- Not a brute force engine  
- Not a “hacking script”  

This is **exposure detection** — the first layer of defensive security visibility.

---

## Future Improvements

Planned evolution of this project:

- Parallel scanning  
- Configurable timeouts  
- Result persistence  
- Exposure change tracking  
- Service banner logging  
- Automation hooks  

This scanner is designed as a base layer for broader security tooling.

---

## Learnings

- Low-level socket programming in C  
- Non-blocking I/O handling  
- OS-level error interpretation  
- Network byte order concepts  
- Building monitoring-oriented utilities  

Most importantly:

**Building systems teaches more than reading about them.**

---

## Author Intent

This project is part of a practical cybersecurity learning path focused on:

- Building tools  
- Understanding networks  
- Thinking in monitoring and visibility  
- Shipping real implementations instead of theory-only work  
