

Module Description

This Loadable Kernel Module create and modify a /proc file to get the system statistic in a /proc file system. 
When this LKM is inserted into kernel, and if you cat or examine the contents of that file, it will provide you with the number of page faults that operating system have handled since it is booted. This problem statement was solved by locating the kernel code that generates page faults statistics. 