Kevin Tsao
Garbage Collector

Originally Homework #4: Garbage Collector
for CS361: Computer Systems Architecture with Professor Cynthia B. Taylor
Memory management, OS protocol, C programming

The purpose of this assignment was, given some region of memory, to perform garbage collection by the mark and sweep method. This is a very conservative version of a garbage collector, built for speed and accuracy. Given an amount of root pointers, the collector must traverse the chunk graph, mark the chunks, and then during the sweep protocol consolidate the usable memory while leaving the non-garbage memory intact.

The skeleton code gave a very primitive version that built the roots of the object graph, in this case the immediate memory chunks reachable from the root, and did nothing with it. Therefore the assignment was to implement the mark and sweep protocol. All of the work was done within the garbage_collector.c file, my additions were all made within the area marked as "ACTUAL WORK" in a few functions. My implementation checks garbage chunks against allocated heap memory pointers, marks garbage chunks using the third lowest order bit, calculates chunk size through pointer math and size calculations to cycle through chunks, and then sweeps using a simple loop with conditionals to prevent tampering with allocated memory.
