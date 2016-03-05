There are two main ways to add code to the Linux kernel. One way is to choose or add code to compile into the kernel during the compilation process. The other method is to add the code to the Linux kernel while it is running, which is what a loadable kernel module is.

For this assignment you will be doing the following:

1. Get the latest Linux kernel source for Ubuntu 14.04.3
2. Compile the latest Linux kernel source for Ubuntu 14.04.3
3. Compile a Hello World kernel module
4. Write your own loadable kernel module to create and modify a /proc file.



Solution

For this assignment I took the latest Linux kernel source and compiled it(took a long time as expected). Then compiled the given hello work kernel module and after gaining the knowledge from the provided hello world kernel module along with research, I wrote my own loadble kernel module to create and modify a /proc file to get and display system statistic in a /proc file system. The loadble kernel module is in /Kernel-hacking/my-LKM-src/ along with the make file and kernel object file.