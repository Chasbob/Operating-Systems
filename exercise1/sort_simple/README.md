# sort_simple

Write a program that reads string from the standard input. All strings will contain no 0-byte characters and will be terminated by a \n newline. The last line of input might not be terminated by a newline character. After reading all strings, these strings should be sorted according of the order of the strcmp function as provided by libc. After all strings have been sorted, these string should be printed to STDOUT in the new order. Every string should be terminated by a \n newline character when writing it to STDOUT.

In this task, there will be at most 20 strings in the input, each one of them at most 20 characters long including the terminating \n newline.

Your solution must be saved in sort.c. You are not allowed to add additional files or change the Makefile.

There is a test in test.sh, that checks your compiled code against a few test cases. Feel free to add more tests. When the tests are passed with success, then test.sh will print "OK".