#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/vmstat.h>
#include <linux/kernel.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");

#define LKM_AUTHOR "Akshay Jain, ajain2@mail.csuchico.edu"
#define LKM_DESC "This is a LKM which get the number of pagefaults"
#define proc_file "num_pagefaults"


static int pagefaults_proc_show(struct seq_file *m, void *v)
{
  
  unsigned long p_Faults[NR_VM_EVENT_ITEMS];
  all_vm_events(p_Faults);
  seq_printf(m,"%lu\n", p_Faults[PGFAULT]);
  return 0;
}


static int pagefaults_proc_open(struct inode *inode, struct file *file)
{
  return single_open(file, pagefaults_proc_show, NULL);
}


static const struct file_operations pagefaults_proc_fops = 
{
  .open = pagefaults_proc_open,
  .read = seq_read,
  .llseek = seq_lseek,
  .release = single_release,
  .owner = THIS_MODULE,
};


static int __init create_entry_init(void)
{
  proc_create(proc_file, 0, NULL, &pagefaults_proc_fops);
  return 0;

}


static void __exit remove_entry_exit(void)
{
  remove_proc_entry(proc_file,NULL);
}


module_init(create_entry_init);
module_exit(remove_entry_exit);
MODULE_AUTHOR(LKM_AUTHOR);
MODULE_DESCRIPTION(LKM_DESC);
MODULE_LICENSE("GPL");


