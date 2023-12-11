# Tomasulo-Algorithm-Simulation
This project is a co-work between Adham El-Asfar and Islam Hassan, and Ahmed Jaheen as part of the Computer Architecture course at 
The American University in Cairo. The project aims to build a simulation for the Tomasulo's Algorithm.  

The goal of this project is to implement an architectural simulator capable of assessing the performance of a simplified out-of-order 16-bit RISC processor that uses Tomasulo’s algorithm without speculation. 

# Assumptions
1. Fetching and decoding take 0 cycles, and the instruction queue is already filled with all the instructions to 
be simulated.
2. No floating-point instructions, registers, or functional units
3. No input/output instructions are supported
4. No interrupts or exceptions are to be handled
5. For each program being executed, assume that the program and its data are fully loaded in the main 
memory
6. There is a one-to-one mapping between reservation stations and functional units. i.e., Each reservation 
station has a functional unit dedicated to it
7. No cache or virtual memory
