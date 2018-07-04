#include <linux/init.h>             // Macros used to mark up functions e.g., __init __exit
#include <linux/module.h>           // Core header for loading LKMs into the kernel
#include <linux/kernel.h>           // Contains types, macros, functions for the kernel
#include <linux/timer.h>

#include "cdp_module.h"
#include "cdp_neighbor.h"
#include "cdp_proc.h"
#include "cdp_receive.h"

static char *name = "cdp";                       ///< An example LKM argument -- default value is "world"
module_param(name, charp, S_IRUGO);              ///< Param desc. charp = char ptr, S_IRUGO can be read/not changed
MODULE_PARM_DESC(name, "The name to display in /var/log/kern.log");  ///< parameter description

rwlock_t cdp_neighbors_rw_lock = __RW_LOCK_UNLOCKED(cdp_neighbors_rw_lock);
struct cdp_neighbor_list *cdp_neighbors;

/** The SNAP OUI (00:00:0C) for Cisco and the protocol Id 0x2000 for CDP */
static unsigned char cdp_snap_id[5] = { 0x0, 0x0, 0x0c, 0x20, 0x00 };

/** Handle to the datalink protocol for CDP */
static struct datalink_proto *cdp_snap_datalink_protocol;

/** A timer for running cleanup tasks and sending packets (TBD) */
static struct timer_list cdp_timer;

/** The interval which should be waited for between running CDP processes */
static const unsigned long cdp_timer_interval_ms = 3000;

void cdp_timer_event_handler( struct timer_list *data )
{
    unsigned long flags;
    int rc;
    struct timespec now;
    
    getnstimeofday(&now);
    
    write_lock_irqsave(&cdp_neighbors_rw_lock, flags);

    cdp_neighbor_list_purge_expired_neighbors(cdp_neighbors, now);

    write_unlock_irqrestore(&cdp_neighbors_rw_lock, flags);

    rc = mod_timer(&cdp_timer, jiffies + msecs_to_jiffies(cdp_timer_interval_ms));
    if(rc < 0)
    {
		printk(KERN_CRIT "cdp: Unable to modify cleanup timer\n" );
        return;
    }
}

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init cdp_module_init(void){
    int rc;

    printk(KERN_INFO "cdp: Hello %s from the Cisco Discovery Protocol module!\n", name);

    cdp_neighbors = cdp_neighbor_list_new();
    if(cdp_neighbors == NULL)
        return -ENOMEM;

    rc = cdp_proc_init();
    if(rc < 0)
    {
        printk("Failed to allocate proc/net/cdp\n");
        cdp_neighbor_list_clean_and_delete(cdp_neighbors);
        return rc;
    }

    timer_setup(&cdp_timer, cdp_timer_event_handler, 0);
    rc = mod_timer(&cdp_timer, jiffies + msecs_to_jiffies(cdp_timer_interval_ms));
    if(rc < 0)
    {
		printk(KERN_CRIT "cdp: Unable to register cleanup timer\n" );
        cdp_proc_exit();
        cdp_neighbor_list_clean_and_delete(cdp_neighbors);
        return -ENOMEM;
    }

    cdp_snap_datalink_protocol = register_snap_client(cdp_snap_id, cdp_receive);

    if (!cdp_snap_datalink_protocol)
    {
		printk(KERN_CRIT "cdp: Unable to register with psnap\n");
        del_timer(&cdp_timer);
        cdp_proc_exit();
        cdp_neighbor_list_clean_and_delete(cdp_neighbors);
        return -ENOMEM;
	}

    return rc;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit cdp_module_exit(void)
{    
	del_timer(&cdp_timer);

    printk(KERN_INFO "cdp: Goodbye %s from the Cisco Discovery Protocol module!\n", name);

    unregister_snap_client(cdp_snap_datalink_protocol);
	cdp_snap_datalink_protocol = NULL;

    cdp_proc_exit();

    cdp_neighbor_list_clean_and_delete(cdp_neighbors);
}

/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(cdp_module_init);
module_exit(cdp_module_exit);

MODULE_LICENSE("GPL");                                  ///< The license type -- this affects runtime behavior
MODULE_AUTHOR("Darren R. Starr 2017 Telenor Inpli AS");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("Cisco Discovery Protocol");         ///< The description -- see modinfo
MODULE_VERSION("0.1");                                  ///< The version of the module