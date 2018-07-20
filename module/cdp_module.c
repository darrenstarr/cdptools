#include <linux/init.h>             // Macros used to mark up functions e.g., __init __exit
#include <linux/module.h>           // Core header for loading LKMs into the kernel
#include <linux/kernel.h>           // Contains types, macros, functions for the kernel
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/inetdevice.h>

#include "cdp_module.h"
#include "cdp_proc.h"
#include "cdp_receive.h"
#include "cdp_transmit.h"
#include "../libcdp/cdp_software_version_string.h"
#include "../libcdp/cdp_packet.h"

static char *name = "cdp";                       ///< An example LKM argument -- default value is "world"
module_param(name, charp, S_IRUGO);              ///< Param desc. charp = char ptr, S_IRUGO can be read/not changed
MODULE_PARM_DESC(name, "The name to display in /var/log/kern.log");  ///< parameter description

rwlock_t cdp_neighbors_rw_lock = __RW_LOCK_UNLOCKED(cdp_neighbors_rw_lock);
struct cdp_neighbor_list *cdp_neighbors;
char *cdp_software_version_string = NULL;
char *cdp_device_id_string = NULL;
/*const*/ uint8_t cdp_multicast_address[] = { 0x01, 0x00, 0x0C, 0xCC, 0xCC, 0xCC };    

/** The SNAP OUI (00:00:0C) for Cisco and the protocol Id 0x2000 for CDP */
static unsigned char cdp_snap_id[5] = { 0x0, 0x0, 0x0c, 0x20, 0x00 };

/** Handle to the datalink protocol for CDP */
struct datalink_proto *cdp_snap_datalink_protocol;

/** A timer for running cleanup tasks and sending packets (TBD) */
static struct timer_list cdp_timer;

/** The interval which should be waited for between running CDP processes */
static const unsigned long cdp_timer_interval_ms = 1000;

/** The interval which should be waited for between transmitting CDP frames */
static const unsigned long cdp_transmit_interval_ms = 5000;

/** The time when the last CDP packet was transmitted */
static struct timespec last_frame_transmitted = { 0, 0 }; 

static void cdp_timer_event_handler(
    //unsigned long data 
    struct timer_list *data
)
{
    unsigned long flags;
    int rc;
    struct timespec now;
    
    getnstimeofday(&now);

    write_lock_irqsave(&cdp_neighbors_rw_lock, flags);

    cdp_neighbor_list_purge_expired_neighbors(cdp_neighbors, now);

    write_unlock_irqrestore(&cdp_neighbors_rw_lock, flags);

    if(((now.tv_sec - last_frame_transmitted.tv_sec) * 1000) >= cdp_transmit_interval_ms)
    {
        //printk("Send me now\n");
        cdp_transmit_packets();
        last_frame_transmitted = now;
    }

    rc = mod_timer(&cdp_timer, jiffies + msecs_to_jiffies(cdp_timer_interval_ms));
    if(rc < 0)
    {
		printk(KERN_CRIT "cdp: Unable to modify cleanup timer\n" );
        return;
    }
}

/** Registers the CDP multicast receiver address to all Ethernet adapters on the system.
  *  @return 0 on success, a negative value on error.
  */
static int __init register_cdp_multicast(void)
{
    struct net_device *dev;

    read_lock(&dev_base_lock);

    dev = first_net_device(&init_net);
    while (dev) {
        int rc;

        if(dev->type == ARPHRD_ETHER)
        {
            rc = dev_mc_add_global(dev, cdp_multicast_address);
            if(rc == 0)
                printk(KERN_INFO "cdp: registered 01:00:0C:CC:CC:CC on interface %s\n", dev->name);
            else
                printk(KERN_INFO "cdp: failed to register 01:00:0C:CC:CC:CC on interface %s\n", dev->name);
        }

        dev = next_net_device(dev);
    }

    read_unlock(&dev_base_lock);

    return 0;
}

/** Deregisters the CDP multicast receiver address from all Ethernet adapters on the system.
  *  @return 0 on success, a negative value on error.
  */
static int unregister_cdp_multicast(void)
{
    struct net_device *dev;

    read_lock(&dev_base_lock);

    dev = first_net_device(&init_net);
    while (dev) {
        int rc;

        if(dev->type == ARPHRD_ETHER)
        {
            rc = dev_mc_del_global(dev, cdp_multicast_address);
            if(rc == 0)
                printk(KERN_INFO "cdp: deregistered 01:00:0C:CC:CC:CC from interface %s\n", dev->name);
            else
                printk(KERN_INFO "cdp: failed to deregister 01:00:0C:CC:CC:CC from interface %s\n", dev->name);
        }

        dev = next_net_device(dev);
    }

    read_unlock(&dev_base_lock);

    return 0;
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

    /* Initialize the platform software version string for transmitting */
    if (generate_cdp_software_version_string(&cdp_software_version_string) < 0)
	{
		printk(KERN_CRIT "cdp: Failed to generate software version\n");
		return -1;
	}
    printk(KERN_INFO "cdp:\n%s\n",cdp_software_version_string);

    /* Initialize the platform device ID string for transmitting */
    if (generate_cdp_device_id_string(&cdp_device_id_string) < 0)
    {
        printk(KERN_CRIT "cdp: Failed to generate the device ID string\n");
        kfree(cdp_software_version_string);

        return -1;
    }
    printk(KERN_INFO "cdp: device ID: %s\n", cdp_device_id_string);

    /* Initialize the CDP neighbor list for storing CDP data */
    cdp_neighbors = cdp_neighbor_list_new();
    if(cdp_neighbors == NULL)
    {
        kfree(cdp_software_version_string);
        kfree(cdp_device_id_string);
        return -ENOMEM;
    }

    /* Create the /proc/net/cdp file system */
    rc = cdp_proc_init();
    if(rc < 0)
    {
        printk(KERN_CRIT "cdp: Failed to allocate proc/net/cdp\n");
        cdp_neighbor_list_clean_and_delete(cdp_neighbors);
        kfree(cdp_software_version_string);
        kfree(cdp_device_id_string);
        return rc;
    }

    /* Register a time to process cleanup events */
    timer_setup(
    //setup_timer(
        &cdp_timer,
        cdp_timer_event_handler,
        0
    );
    
    rc = mod_timer(&cdp_timer, jiffies + msecs_to_jiffies(cdp_timer_interval_ms));
    if(rc < 0)
    {
		printk(KERN_CRIT "cdp: Unable to register cleanup timer\n" );
        cdp_proc_exit();
        cdp_neighbor_list_clean_and_delete(cdp_neighbors);
        kfree(cdp_software_version_string);
        kfree(cdp_device_id_string);
        return -ENOMEM;
    }

    /* Register the packet receive handler for incoming CDP packets */
    cdp_snap_datalink_protocol = register_snap_client(cdp_snap_id, cdp_receive);
    if (!cdp_snap_datalink_protocol)
    {
		printk(KERN_CRIT "cdp: Unable to register with psnap\n");
        del_timer(&cdp_timer);
        cdp_proc_exit();
        cdp_neighbor_list_clean_and_delete(cdp_neighbors);
        kfree(cdp_software_version_string);
        kfree(cdp_device_id_string);
        return -ENOMEM;
	}

    /* Register the CDP MAC address on the interfaces so the Ethernet MAC will permit the frames */
    register_cdp_multicast();

    return rc;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit cdp_module_exit(void)
{    
	del_timer(&cdp_timer);

    unregister_cdp_multicast();

    printk(KERN_INFO "cdp: Goodbye %s from the Cisco Discovery Protocol module!\n", name);

    unregister_snap_client(cdp_snap_datalink_protocol);
	cdp_snap_datalink_protocol = NULL;

    cdp_proc_exit();

    cdp_neighbor_list_clean_and_delete(cdp_neighbors);

    kfree(cdp_device_id_string);

    kfree(cdp_software_version_string);
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
