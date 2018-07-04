#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <net/net_namespace.h>

#include "cdp_module.h"
#include "cdp_proc.h"

/** The top level proc directory entry for CDP (/proc/net/cdp) */
static struct proc_dir_entry *cdp_proc_dir;

/** The proc directory entry (/proc/net/cdp/summary) */
static struct proc_dir_entry *cdp_summary_proc_entry;

/** The proc directory entry (/proc/net/cdp/detail) */
static struct proc_dir_entry *cdp_detail_proc_entry;

/** The proc directory entry (/proc/net/cdp/json) */
static struct proc_dir_entry *cdp_json_proc_entry;

/** proc_fs sequential file system handler for iterating the CDP entries start function.
  *  This function is called by the system with the starting index for this pass. If
  *  the index is invalid (past the end) then it simply returns zero.
  *
  *  @param seq the handle to the sequential file structure.
  *  @param pos a pointer to the position/index in the array.
  *  @return a pointer to a CDP neighbor entry or NULL.
  */
static void *cdp_seq_start(struct seq_file *seq, loff_t *pos)
{
    unsigned long *cdp_neighbors_rw_lock_flags = (unsigned long *)kmalloc(sizeof(unsigned long), GFP_ATOMIC);
    seq->private = cdp_neighbors_rw_lock_flags;

    read_lock_irqsave(&cdp_neighbors_rw_lock, *cdp_neighbors_rw_lock_flags);

    if(cdp_neighbors == NULL)
        return NULL;

    return cdp_neighbor_list_get_by_index(cdp_neighbors, (int)*pos);
}

/** proc_fs sequential file system handler for iterating the CDP entries next function
  *  This function is called to iterate to the next item in the CDP entry list.
  *  When there is nothing left to iterate to, the function returns NULL to say so.
  *  Since NULL can also signify that the process is willing to give up time by
  *  returning NULL, cdp_seq_start will be called again following the end of the list.
  *
  *  @param seq the handle to the sequential file structure.
  *  @param v the value to iterate forward from. This is the last "show"n value.
  *  @param pos the iterator index to be updated.
  *  @return the next CDP neighbor entry if it's available or NULL if the end is reached.
  */
static void *cdp_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
    struct cdp_neighbor *neighbor = (struct cdp_neighbor *)v;
    ++*pos;

    if(neighbor == NULL)
        return NULL;

    return neighbor->next;
}

/** proc_fs sequential file system handler for iterating the CDP entries stop function
  *  This function is called at the end of a sequence of iterating over the entries
  *  in the list. It is responsible for releasing any associated thread synchronization
  *  locks.
  * 
  *  @param seq the handle to the sequential file structure.
  *  @param v the value of the last item iterated to.
  */
static void cdp_seq_stop(struct seq_file *seq, void *v)
{
    unsigned long *cdp_neighbors_rw_lock_flags = (unsigned long *)seq->private;

    read_unlock_irqrestore(&cdp_neighbors_rw_lock, *cdp_neighbors_rw_lock_flags);

    kfree(cdp_neighbors_rw_lock_flags);
}

static int cdp_seq_show(struct seq_file *seq, void *v)
{
    //seq_printf(seq, "%s\n--\n", seq->file->f_path.dentry->d_iname);

    if(!strcmp(seq->file->f_path.dentry->d_iname, "summary"))
        return cdp_seq_summary_show(seq, v);
    else if(!strcmp(seq->file->f_path.dentry->d_iname, "detail"))
        return cdp_seq_detail_show(seq, v);
    else if(!strcmp(seq->file->f_path.dentry->d_iname, "json"))
        return cdp_seq_json_show(seq, v);

    printk("Attempting to read an unknown /proc/net/cdp file (%s)\n", seq->file->f_path.dentry->d_iname);
    return -1;
}

/** Sequential file system handler entry points for the /proc/net/cdp/ files */
static const struct seq_operations cdp_seq_ops = {
	.start  = cdp_seq_start,
	.next   = cdp_seq_next,
	.stop   = cdp_seq_stop,
	.show   = cdp_seq_show,
};

/** The inode open handler for the /proc/net/cdp/ files
  *  @param inode the inode structure to provide entry points for.
  *  @param file the file to provide entry points for processing to.
  *  @return 0 on success, negative values on failure.
  */
static int cdp_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &cdp_seq_ops);
}

/** The proc_fs inode entry points for processing the /proc/net/cdp/ files */
static const struct file_operations cdp_seq_fops = {
	.open		= cdp_seq_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

int __init cdp_proc_init(void)
{
    cdp_proc_dir = proc_mkdir("cdp", init_net.proc_net);
    if(cdp_proc_dir == NULL)
    {
        return -ENOMEM;
    }

    cdp_summary_proc_entry = proc_create("summary", 0444, cdp_proc_dir, &cdp_seq_fops);
	if (!cdp_summary_proc_entry)
    {
        remove_proc_entry("cdp", init_net.proc_net);
        return -ENOMEM;
    }

    cdp_detail_proc_entry = proc_create("detail", 0444, cdp_proc_dir, &cdp_seq_fops);
	if (!cdp_detail_proc_entry)
    {
        remove_proc_entry("summary", cdp_proc_dir);
        remove_proc_entry("cdp", init_net.proc_net);
        return -ENOMEM;
    }

    cdp_json_proc_entry = proc_create("json", 0444, cdp_proc_dir, &cdp_seq_fops);
	if (!cdp_json_proc_entry)
    {
        remove_proc_entry("detail", cdp_proc_dir);
        remove_proc_entry("summary", cdp_proc_dir);
        remove_proc_entry("cdp", init_net.proc_net);
        return -ENOMEM;
    }

    return 0;
}

void cdp_proc_exit(void)
{
    remove_proc_entry("json", cdp_proc_dir);
    remove_proc_entry("detail", cdp_proc_dir);
    remove_proc_entry("summary", cdp_proc_dir);
    remove_proc_entry("cdp", init_net.proc_net);
}
