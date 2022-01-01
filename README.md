# Dynamic Memory Allocator

A dynamic memory allocator created with intent to replace the provided memory allocation functions in C.

The implementation is a multi-pool allocator that provides a pool allocation implementation for all allocations between 1 and 4088 bytes, and bulk allocation for anything greater.

