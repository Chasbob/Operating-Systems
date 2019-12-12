#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for KERN_ALERT */
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/compiler.h>
#include <net/tcp.h>
#include <linux/namei.h>
#include <linux/version.h>
#include <linux/proc_fs.h>

MODULE_AUTHOR("Eike Ritter <E.Ritter@cs.bham.ac.uk>");
MODULE_DESCRIPTION("Extensions to the firewall");
MODULE_LICENSE("GPL");

/* make IP4-addresses readable */

#define PROC_ENTRY_FILENAME "kernelWrite"

DECLARE_RWSEM(counter_sem); /* semaphore to protect counter access */

#define BUFFERSIZE 80
#define NIPQUAD(addr)              \
  ((unsigned char *)&addr)[0],     \
      ((unsigned char *)&addr)[1], \
      ((unsigned char *)&addr)[2], \
      ((unsigned char *)&addr)[3]

struct nf_hook_ops *reg;
static struct proc_dir_entry *Our_Proc_File;

// the firewall hook - called for each outgoing packet
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 3, 0)
#error "Kernel version < 4.4 not supported!"
//kernels < 4.4 need another firewallhook!
#endif
unsigned int FirewallExtensionHook(void *priv,
                                   struct sk_buff *skb,
                                   const struct nf_hook_state *state)
{

  struct path path;
  struct tcphdr *tcp;
  struct tcphdr _tcph;
  struct sock *sk;
  struct mm_struct *mm;
  struct dentry *procDentry;
  struct dentry *parent;
  pid_t mod_pid;
  char cmdlineFile[BUFFERSIZE];
  int res;
  sk = skb->sk;
  if (!sk)
  {
    printk(KERN_INFO "firewall: netfilter called with empty socket!\n");
    ;
    return NF_ACCEPT;
  }

  if (sk->sk_protocol != IPPROTO_TCP)
  {
    printk(KERN_INFO "firewall: netfilter called with non-TCP-packet.\n");
    return NF_ACCEPT;
  }

  /* get the tcp-header for the packet */
  tcp = skb_header_pointer(skb, ip_hdrlen(skb), sizeof(struct tcphdr), &_tcph);
  if (!tcp)
  {
    printk(KERN_INFO "Could not get tcp-header!\n");
    return NF_ACCEPT;
  }
  if (tcp->syn)
  {
    struct iphdr *ip;
    mod_pid = current->pid;
    snprintf(cmdlineFile, BUFFERSIZE, "/proc/%d/exe", mod_pid);
    res = kern_path(cmdlineFile, LOOKUP_FOLLOW, &path);
    printk(KERN_INFO "firewall: Starting connection \n");
    procDentry = path.dentry;
    parent = procDentry->d_parent;
    printk(KERN_INFO "the name is %s\n", procDentry->d_name.name);
    printk(KERN_INFO "the name of the parent is %s\n", parent->d_name.name);
    ip = ip_hdr(skb);
    if (!ip)
    {
      printk(KERN_INFO "firewall: Cannot get IP header!\n!");
    }
    else
    {
      printk(KERN_INFO "firewall: Destination address = %u.%u.%u.%u\n", NIPQUAD(ip->daddr));
    }
    printk(KERN_INFO "firewall: destination port = %d\n", ntohs(tcp->dest));

    if (in_irq() || in_softirq() || !(mm = get_task_mm(current)))
    {
      printk(KERN_INFO "Not in user context - retry packet\n");
      return NF_ACCEPT;
    }
    mmput(mm);

    // if (ntohs(tcp->dest) == 80)
    // {
    //   tcp_done(sk); /* terminate connection immediately */
    //   printk(KERN_INFO "Connection shut down\n");
    //   return NF_DROP;
    // }
  }
  return NF_ACCEPT;
}

char *ins(const char *buffer, int length)
{
    char *temp;
    size_t i;
    temp = kmalloc(length, GFP_KERNEL);
    if (temp == NULL)
    {
        printk(KERN_INFO "charDeviceDriver - newNode->key: kmalloc returned NULL\n");
        return NULL;
    }
    for (i = 0; i < length; i++)
    {
        printk ("{%c}",*buffer);
        *(temp + i) = *(buffer + i);
    }
    return temp;
}

ssize_t kernelWrite(struct file *file, const char __user *buffer, size_t count, loff_t *offset)
{

  char* command;
  // int res;
  printk(KERN_INFO "kernelWrite entered\n");
  command = ins(buffer, count);
  printk(KERN_INFO "kernelWrite: size: {%i}, command: {%.*s}\n", count, count, command);
  switch (*command)
  {
  case 'L':
    printk(KERN_INFO "List rules\n");
    break;
  case 'W':
    printk(KERN_INFO "Write rules from file %.*s\n",count - 2,command + 2);
    break;
  default:
    printk(KERN_INFO "kernelWrite: Illegal command \n");
  }
  return count;
}
static struct nf_hook_ops firewallExtension_ops = {
    .hook = FirewallExtensionHook,
    .pf = PF_INET,
    .priority = NF_IP_PRI_FIRST,
    .hooknum = NF_INET_LOCAL_OUT};

int procfs_open(struct inode *inode, struct file *file)
{
  printk(KERN_INFO "kernelWrite opened\n");
  try_module_get(THIS_MODULE);
  return 0;
}

/* 
 * The file is closed - again, interesting only because
 * of the reference count. 
 */
int procfs_close(struct inode *inode, struct file *file)
{
  printk(KERN_INFO "kernelWrite closed\n");
  module_put(THIS_MODULE);
  return 0; /* success */
}

const struct file_operations File_Ops_4_Our_Proc_File = {
    .owner = THIS_MODULE,
    .write = kernelWrite,
    .open = procfs_open,
    .release = procfs_close,
};
int init_module(void)
{

  int errno;

  /* create the /proc file */
  Our_Proc_File = proc_create_data(PROC_ENTRY_FILENAME, 0644, NULL, &File_Ops_4_Our_Proc_File, NULL);

  /* check if the /proc file was created successfuly */
  if (Our_Proc_File == NULL)
  {
    printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
           PROC_ENTRY_FILENAME);
    return -ENOMEM;
  }

  printk(KERN_INFO "/proc/%s created\n", PROC_ENTRY_FILENAME);

  errno = nf_register_hook(&firewallExtension_ops); /* register the hook */
  if (errno)
  {
    printk(KERN_INFO "Firewall extension could not be registered!\n");
  }
  else
  {
    printk(KERN_INFO "Firewall extensions module loaded\n");
  }
  return 0; /* success */
}

void cleanup_module(void)
{

  remove_proc_entry(PROC_ENTRY_FILENAME, NULL);
  printk(KERN_INFO "/proc/%s removed\n", PROC_ENTRY_FILENAME);

  printk(KERN_INFO "kernelWrite:Proc module unloaded.\n");
  nf_unregister_hook(&firewallExtension_ops); /* restore everything to normal */
  printk(KERN_INFO "Firewall extensions module unloaded\n");
}