CS 3502 Project 1

Requirements:
- GCC
- POSIX Threads (pthread)
- Linux/Ubuntu

Compile:

Phase 1:
gcc -Wall -Wextra -pthread phase1.c -o phase1

Phase 2:
gcc -Wall -Wextra -pthread phase2.c -o phase2

Phase 3:
gcc -Wall -Wextra -pthread phase3.c -o phase3

Phase 4:
gcc -Wall -Wextra -pthread phase4.c -o phase4

Run:

Phase 1:
./phase1

Phase 2:
./phase2

Phase 3:
./phase3

Phase 4:
./phase4

For Phase 3, the program is expected to deadlock. To stop it automatically after a few seconds, run:

timeout 5s ./phase3

Output can be saved with:

./phase1 > phase1_output.txt
./phase2 > phase2_output.txt
timeout 5s ./phase3 > phase3_output.txt
./phase4 > phase4_output.txt
