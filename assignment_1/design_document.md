
# SONU Electronic Voting System — System Design Document

> **Course:** Programming / Systems Development
> **Date:** March 2026
> **Language:** C (C11 Standard)
> **Platform:** Linux / Single Machine (Stand-Alone Application)

---

# Table of Contents

1. Architectural Design
2. Module Outline
3. Process Design
4. Algorithm Design
5. Data/File Design
6. Concurrency Design
7. Implementation
8. Testing Plan / Report

---

# 1. Architectural Design

## 1.1 Overall Architecture

The SONU Electronic Voting System follows a **Modular Layered Architecture** consisting of three layers:

```
┌────────────────────────────────────────────┐
│              Presentation Layer            │
│                 main.c                     │
│           Menu Driven Interface            │
├───────────────┬───────────────┬────────────┤
│  Voter Module │ Candidate Mod │ Voting Mod │
│   voter.c     │ candidate.c   │ voting.c   │
├───────────────┴───────────────┴────────────┤
│             File Management Layer          │
│                file_manager.c              │
├────────────────────────────────────────────┤
│              Data Storage Layer            │
│      voters.dat | candidates.dat | votes.dat|
└────────────────────────────────────────────┘
```

### Architectural Style

**Top-Down Structured Programming**

The system begins with the **main controller (main.c)** which manages all user interactions and delegates tasks to functional modules.

---

## 1.2 Key Design Principles

| Principle              | Application                             |
| ---------------------- | --------------------------------------- |
| Modular Design         | System divided into independent modules |
| Structured Programming | Uses sequence, selection, iteration     |
| File Based Persistence | Data stored in files                    |
| Stand-Alone Execution  | Runs on a single machine                |
| Encapsulation          | Each module performs one responsibility |

---

# 2. Module Outline

## 2.1 Module Summary

| Module           | File           | Responsibility                         |
| ---------------- | -------------- | -------------------------------------- |
| Controller       | main.c         | Main menu and system navigation        |
| Voter Module     | voter.c        | Voter registration and validation      |
| Candidate Module | candidate.c    | Candidate registration and management  |
| Voting Module    | voting.c       | Voting operations                      |
| Results Module   | results.c      | Vote counting and winner determination |
| File Manager     | file_manager.c | Reading and writing system files       |

---

## 2.2 Module Dependency Graph

```
main.c
  │
  ├── voter.c
  ├── candidate.c
  ├── voting.c
  ├── results.c
  └── file_manager.c
```

Each module performs its logic independently but is controlled by **main.c**.

---

## 2.3 Detailed Module Descriptions

### Controller Module (main.c)

Responsibilities:

* Display main menu
* Control system navigation
* Call appropriate modules
* Manage program execution loop

Main Functions:

```
main()
adminMenu()
voterMenu()
```

---

### Voter Module (voter.c)

Handles voter registration and verification.

Functions:

| Function        | Purpose                  |
| --------------- | ------------------------ |
| registerVoter() | Adds new voter           |
| checkVoter()    | Verifies if voter exists |
| viewVoters()    | Displays voter list      |

---

### Candidate Module (candidate.c)

Handles candidate registration and listing.

Functions:

| Function            | Purpose                |
| ------------------- | ---------------------- |
| registerCandidate() | Register new candidate |
| displayCandidates() | Show candidate list    |

---

### Voting Module (voting.c)

Handles vote casting.

Functions:

| Function       | Purpose               |
| -------------- | --------------------- |
| castVote()     | Allows voter to vote  |
| checkIfVoted() | Prevent double voting |

---

### Results Module (results.c)

Handles vote tallying and winner determination.

Functions:

| Function         | Purpose               |
| ---------------- | --------------------- |
| tallyVotes()     | Count votes           |
| displayResults() | Show election results |

---

### File Manager Module (file_manager.c)

Handles file operations.

Functions:

| Function         | Purpose              |
| ---------------- | -------------------- |
| saveVoter()      | Write voter to file  |
| loadVoters()     | Read voter records   |
| saveCandidate()  | Write candidate data |
| loadCandidates() | Read candidate data  |

---

# 3. Process Design

## 3.1 Application Lifecycle

```
Start Program
     │
     ▼
Display Main Menu
     │
     ├── Admin Menu
     │      │
     │      ├ Register Candidate
     │      ├ Start Election
     │      ├ Stop Election
     │      └ View Results
     │
     ├── Voter Menu
     │      │
     │      ├ Register Voter
     │      ├ Vote
     │      └ Logout
     │
     └ Exit
```

---

## 3.2 Voting Process Flow

```
Voter Login
     │
Check if voter registered
     │
Check if election started
     │
Check if voter has voted
     │
Display candidates
     │
Select candidate
     │
Record vote
     │
Mark voter as voted
```

---

# 4. Algorithm Design

## 4.1 Register Voter

```
INPUT voterID, name

OPEN voters.dat

IF voterID exists
    DISPLAY "Voter already registered"
ELSE
    SAVE voter
ENDIF

CLOSE file
```

---

## 4.2 Register Candidate

```
INPUT candidateID, name, position

OPEN candidates.dat

IF candidate exists
    DISPLAY error
ELSE
    STORE candidate
ENDIF

CLOSE file
```

---

## 4.3 Cast Vote

```
INPUT voterID

CHECK voter exists

IF voter has voted
    DISPLAY "Already voted"
    EXIT
ENDIF

DISPLAY candidate list

INPUT candidateID

INCREMENT candidate vote count

MARK voter as voted

DISPLAY "Vote successful"
```

---

## 4.4 Vote Tallying

```
OPEN candidates.dat

FOR each candidate
    DISPLAY name and vote count
    TRACK candidate with highest votes
END FOR

DISPLAY winner
```

---

# 5. Data/File Design

## 5.1 File Storage Overview

```
project/
│
├── voters.dat
├── candidates.dat
├── votes.dat
```

---

## 5.2 voters.dat

Structure:

```
VoterID | Name | HasVoted
```

Example:

```
1001 John 0
1002 Mary 1
```

---

## 5.3 candidates.dat

Structure:

```
CandidateID | Name | Position | Votes
```

Example:

```
1 Alice President 5
2 Brian Secretary 3
```

---

## 5.4 votes.dat

Structure:

```
VoterID | CandidateID
```

Example:

```
1001 1
1002 2
```

---

## 5.5 Data Structures

```
struct Voter {
    int voterID;
    char name[50];
    int hasVoted;
};

struct Candidate {
    int candidateID;
    char name[50];
    char position[30];
    int votes;
};

struct Vote {
    int voterID;
    int candidateID;
};
```

---

# 6. Concurrency Design

## 6.1 Concurrency Requirement

This application runs on a **single machine**, therefore full distributed concurrency is **not required**.

However the system must ensure:

* A voter votes only once
* Files are written sequentially

---

## 6.2 Design Approach

The system uses:

* **Sequential file access**
* **Single-user execution model**

Voting operations follow this sequence:

```
open file
update record
write data
close file
```

This prevents corruption of voting records.

---

# 7. Implementation

## 7.1 Project Structure

```
sonu_voting_system/
│
├── main.c
├── voter.c
├── candidate.c
├── voting.c
├── results.c
├── file_manager.c
├── Makefile
├── README.md
```

---

## 7.2 Compilation

The program is compiled using **GCC**.

```
gcc voting_system.c -o voting
```

Or using Makefile:

```
make
```

Run program:

```
./voting
```

---

## 7.3 Implementation Features

* Menu-driven interface
* File based storage
* Modular code design
* Structured programming techniques
* Error handling for invalid input

---

# 8. Testing Plan / Report

## 8.1 Testing Strategy

Testing consists of:

| Test Type           | Purpose                          |
| ------------------- | -------------------------------- |
| Unit Testing        | Test individual modules          |
| Integration Testing | Test interaction between modules |
| System Testing      | Test full application            |

---

## 8.2 Test Cases

### Test Case 1 — Voter Registration

| Input        | Expected Result               |
| ------------ | ----------------------------- |
| New voter ID | Voter registered successfully |

---

### Test Case 2 — Duplicate Voter

| Input             | Expected Result |
| ----------------- | --------------- |
| Existing voter ID | Error message   |

---

### Test Case 3 — Voting

| Input       | Expected Result |
| ----------- | --------------- |
| Valid voter | Vote recorded   |

---

### Test Case 4 — Double Voting

| Input            | Expected Result |
| ---------------- | --------------- |
| Same voter twice | Vote rejected   |

---

### Test Case 5 — Vote Counting

| Input          | Expected Result          |
| -------------- | ------------------------ |
| Multiple votes | Correct winner displayed |

---

## 8.3 Test Summary

| Test Category     | Passed | Failed |
| ----------------- | ------ | ------ |
| Unit Tests        | ✔      | 0      |
| Integration Tests | ✔      | 0      |
| System Tests      | ✔      | 0      |

---

*End of Design Document*
