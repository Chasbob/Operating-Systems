# The task

Write an extension to the linux firewall which makes it possible to specify which programs are allowed use which outgoing port.

More precisely, you should write a user space program and a kernel module.

## Firewall rules

A firewall rule consists of a port number and a filename (the full path) of a program separated by a space, meaning that the corresponding program is allowed to make outgoing connections on this TCP-port. If there is no rule for a given port, any program should be allowed to make outgoing connections on this port. A connection is not allowed when rules for the port exist, but the program trying to establish the connection is not in the list of allowed programs. If a connection is not allowed, it should be immediately terminated.

The kernel module processes the packets and maintains the firewall rules, and displays the firewall rules via _printk_ in _/var/log/kern.log_. The output should be:

Firewall rule: _⟨port ⟩ ⟨program⟩_

For every rule that is configured, _⟨port⟩_ is the port number in decimal representation and _⟨program⟩_ is the full path to the executeable. When the kernel module is unloaded, the firewall extensions should be deleted.

## User space configuration

The user space program, which must be called _firewallSetup_, has commands firstly for triggering the listing of the firewall rules in _/var/log/kern.log_, and secondly for setting the firewall rules. A new set of firewall rules overrides the old set (no appending). You should use the file _/proc/firewallExtension_ for communication between the user program and the kernel.

If replacing the set of firewall rules fails for any reason, the old set of firewall rules should be retained.

To make marking easier, there should be two ways of calling the user space program. The first one is

firewallSetup L

This way of calling the user space program causes the firewall rules to be displayed in _/var/log/kern.log_ as specified above.

The second way of calling the program is

firewallSetup W _⟨filename⟩_

where _⟨filename⟩_ is the name of the file containing the firewall rules. This file contains one firewall rule per line. _firewallSetup_ should check whether the filename in the firewall rule denotes an existing executable file. If there is any error in the syntax or any filename does not an executable file, this program should abort with the messages _ERROR: Ill-formed file_ and _ERROR: Cannot execute file_ respectively.

## Submission

You should put all your files into a directory named exercise4 in your project on the School's git server. The directory exercise4 must be at the top level of your project for the marking scripts to work. The directory exercise4 should contain a Makefile which builds the kernel module which must be called firewallExtension.ko. This directory must also contain a subdirectory called Setup. This subdirectory must contain a Makefile which builds the firewallSetup-binary. We will run the command make in the directory exercise4 in order to obtain the kernel module, and we will run make in the subdirectory Setup to obtain the FirewallSetup-binary. Do not use any files or directory with the prefix "test", since we will use such files for running tests. Submission consists of pushing your solution to your project as specified above; the last submission before the submission deadline counts as your submission. It is worth trying out submission well before the deadline even if the solution is not correct yet. Because of differences between kernel versions it is important that you use the provided kernel version, which is 4.4.0-93-generic.

## General Coding

Your kernel code may assume that only well-formed files are written by firewallSetup. Only one process should be allowed to open the _/proc/firewallExtension_ file at any give time. If a second process tries to open this file it should receive the error message _-EAGAIN_. You need to ensure that you handle concurrency in the kernel correctly. In particular, any number of outgoing connections may be started at any time, hence several instances of the procedures handling the packets may be executed at the same time. It is very important that you maximize the degree of parallelism. In particular, your critical sections should be as short as possible. We recommend using the APIS for linked lists, locking and the Basic C Library functions provided by the kernel. Please be careful to check all buffer boundaries. Do not modify or add files or directories that start with _test_ since we will use those files to test your project. We have provided a test script for development in the file _test.sh_. For marking we will use additional, more advanced, test scripts which check whether your program satisfies the specification. If the provided test scripts fail, all the more advanced test scripts are likely to fail as well. Any submission which does not compile will be awarded 0 marks and will not be reviewed.