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

#define PROC_ENTRY_FILENAME "firewallExtension"

DECLARE_RWSEM(counter_sem); /* semaphore to protect counter access */
#define BUFFERSIZE 80
#define NIPQUAD(addr)              \
  ((unsigned char *)&addr)[0],     \
      ((unsigned char *)&addr)[1], \
      ((unsigned char *)&addr)[2], \
      ((unsigned char *)&addr)[3]

typedef struct firewallRule
{
  struct firewallRule *next;
  int port;
  struct path *exe;
} rule;

rule *head;
struct nf_hook_ops *reg;
static struct proc_dir_entry *Our_Proc_File;

// the firewall hook - called for each outgoing packet
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 3, 0)
#error "Kernel version < 4.4 not supported!"
//kernels < 4.4 need another firewallhook!
#endif
// Push rule to list
void push(int port, struct path *exe)
{
  rule *c = head;
  printk(KERN_INFO "push: c={%p}", c);
  printk(KERN_INFO "push: c.port={%d},c.exe={%p},c.next={%p}", c->port, c->exe, c->next);
  if (c->port == 0)
  {
    c->port = port;
    c->exe = exe;
    return;
  }
  while (c->next != NULL)
  {
    c = c->next;
  }
  /* now we can add a new variable */
  c->next = kmalloc(sizeof(rule), GFP_KERNEL);
  c->next->port = port;
  c->next->exe = exe;
  c->next->next = NULL;
}

void print_list(void)
{
  char buffer[BUFFERSIZE];
  char* path;
  rule *c = head;
  printk(KERN_INFO "print: c={%p}", c);
  printk(KERN_INFO "print: c.port={%d}, c.exe={%p}, c.next={%p}", c->port, c->exe, c->next);
  while (c != NULL)
  {
    path = dentry_path_raw(c->exe->dentry, buffer, BUFFERSIZE);
    printk(KERN_INFO "RULE: port => {%d}, exe => {%s}", c->port, path);
    c = c->next;
  }
}
int allowed(int port, struct path path){

  char buffer[BUFFERSIZE];
  rule *c = head;
  printk(KERN_INFO "allowed: port => {%d}, path => {%s}\n",port,dentry_path_raw(c->exe->dentry, buffer, BUFFERSIZE));
  while (c != NULL)
  {
    printk(KERN_INFO "\n");
    if(path.dentry->d_inode == c->exe->dentry->d_inode){
      if(c->port == port){
        return 1;
      }
    }
    c = c->next;
  }

  return -1;
}
void addRule(char *raw, int size)
{
  char *portStr;
  char *exeStr;
  int portLen;
  int exeLen;
  int index = 0;
  int pathResult;
  struct path *path;
  long port;
  printk(KERN_INFO "addRule: size: {%d}, raw: {%.*s}", size, size, raw);

  // Find the split between port and path
  while (*(raw + index) != ' ' && index < size)
  {
    index++;
  }
  portLen = ++index;
  exeLen = size - index + 1;
  portStr = kmalloc(portLen, GFP_KERNEL);
  exeStr = kmalloc(exeLen, GFP_KERNEL);
  if (portStr == NULL)
  {
    printk(KERN_ERR "kmalloc returned NULL for portStr\n");
  }
  if (exeStr == NULL)
  {
    printk(KERN_ERR "kmalloc returned NULL for exeStr\n");
  }
  snprintf(portStr, portLen, "%.*s", portLen, raw);
  snprintf(exeStr, exeLen, "%.*s", exeLen, raw + index);
  printk(KERN_INFO "port: {%.*s}\n", portLen, portStr);
  printk(KERN_INFO "exe: {%.*s}\n", exeLen, exeStr);
  path = kmalloc(sizeof(struct path), GFP_KERNEL);
  pathResult = kern_path(exeStr, LOOKUP_FOLLOW, path);
  printk(KERN_INFO "pathResult: {%d}\n", pathResult);
  kstrtol(portStr, 10, &port);
  printk(KERN_INFO "port as int: {%lu}\n", port);
  push(port, path);
  kfree(portStr);
  kfree(exeStr);
}

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
    int allow = 0;
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
    allow = allowed(ntohs(tcp->dest), path);
    printk(KERN_INFO "allow = %d\n", allow);
    mmput(mm);
    if (allow == -1){
      tcp_done(sk); /* terminate connection immediately */
      return NF_DROP;
    }
    // if (ntohs(tcp->dest) == 80)
    // {
    //   tcp_done(sk); /* terminate connection immediately */
    //   printk(KERN_INFO "Connection shut down\n");
    //   return NF_DROP;
    // }
  }
  return NF_ACCEPT;
}

char *read_buffer(const char *buffer, int length)
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
    printk("{%c}", *buffer);
    *(temp + i) = *(buffer + i);
  }
  return temp;
}

ssize_t kernelWrite(struct file *file, const char __user *buffer, size_t count, loff_t *offset)
{

  char raw[BUFFERSIZE];
  char *command;
  // int res;
  printk(KERN_INFO "kernelWrite entered\n");
  command = read_buffer(buffer, count);
  printk(KERN_INFO "kernelWrite: size: {%i}, command: {%.*s}\n", count, count, command);
  switch (*command)
  {
  case 'L':
    printk(KERN_INFO "List rules\n");
    print_list();
    break;
  case 'A':
    snprintf(raw, BUFFERSIZE, "%.*s", count - 2, command + 2);
    printk(KERN_INFO "raw: {%.*s}\n", count - 2, raw);
    addRule(raw, count - 2);
    printk(KERN_INFO "Add rule {%.*s}\n", count - 2, command + 2);
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

  head = kmalloc(sizeof(rule), GFP_KERNEL);
  if (head == NULL)
  {
    printk(KERN_INFO "firewall init: kmalloc returned NULL\n");
    return -ENOMEM;
  }
  head->next = NULL;
  head->exe = NULL;
  head->port = 0;
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