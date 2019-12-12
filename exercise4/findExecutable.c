/*  hello.c - The simplest kernel module.
 */

#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for KERN_ALERT */
#include <linux/namei.h>
#include <linux/sched.h>
#include <linux/dcache.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#define BUFFERSIZE 80

MODULE_AUTHOR("Eike Ritter <E.Ritter@cs.bham.ac.uk>");
MODULE_DESCRIPTION("Finding executable");
MODULE_LICENSE("GPL");

int init_module(void)
{
    struct path path;
    pid_t mod_pid;
    struct dentry *procDentry;
    struct dentry *parent;
    struct dentry *pp;
    struct dentry *ppp;
    struct path p1;
    struct path p2;
    char cmdlineFile[BUFFERSIZE];
    char buffer[BUFFERSIZE];
    int res;
    int resP1;
    int resP2;
    int resEQ;
    char* rawPath;
    printk(KERN_INFO "findExecutable module loading\n");
    /* current is pre-defined pointer to task structure of currently running task */
    mod_pid = current->pid;
    snprintf(cmdlineFile, BUFFERSIZE, "/proc/%d/exe", mod_pid);
    res = kern_path(cmdlineFile, LOOKUP_FOLLOW, &path);
    resP1 = kern_path("/usr/bin/firefox", LOOKUP_FOLLOW, &p1);
    resP2 = kern_path("/usr/bin/firefox", LOOKUP_FOLLOW, &p2);
    resEQ = path_equal(&p1, &p1);
    printk("resEQ: %i", resEQ);
    if (res)
    {
        printk(KERN_INFO "Could not get dentry for %s!\n", cmdlineFile);
        return -EFAULT;
    }

    procDentry = path.dentry;
    parent = procDentry->d_parent;
    pp = parent->d_parent;
    ppp = parent->d_parent;
    printk(KERN_INFO "the name is %s\n", procDentry->d_name.name);
    printk(KERN_INFO "the name of the parent is %s\n", parent->d_name.name);
    printk(KERN_INFO "The name of the pp is %s\n", pp->d_name.name);
    printk(KERN_INFO "The name of the ppp is %s\n", ppp->d_name.name);
    rawPath = dentry_path_raw(path.dentry, buffer, BUFFERSIZE);
    printk(KERN_INFO "rawPath => {%s}\n",rawPath);
    path_put(&path);
    return 0;
}

void cleanup_module(void)
{
    printk(KERN_INFO "findExecutable module unloading \n");
}
