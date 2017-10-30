Kevin Tsao
Garbage Collector

Originally Homework #4: Garbage Collector
for CS361: Computer Systems Architecture
Memory management, OS protocol, C programming

The purpose of this assignment was, given some region of memory, to perform garbage collection by the mark and sweep method. This is a very conservative version of a garbage collector, built for speed and accuracy. Given an amount of root pointers, the collector must traverse the chunk graph, mark the chunks, and then during the sweep protocol consolidate the usable memory while leaving the non-garbage memory intact.