#ifndef CDP_PROC_H
#define CDP_PROC_H

#include <linux/seq_file.h>

/** Entry point for initializing the proc_fs entries.
  *  @return 0 on success or a negative value on error.
  */
int __init cdp_proc_init(void);

/** Exit point for tearing down the proc_fs entries. */
void cdp_proc_exit(void);

/** Function to be called to produce printable output for the sequential file /proc/net/cdp/summary
  *  @param seq The handle to the sequential file structure.
  *  @param v A pointer to the current value to print
  *  @return 0 on success or a negative value on failure.
  */
int cdp_seq_summary_show(struct seq_file *seq, void *v);


/** Function to be called to produce printable output for the sequential file /proc/net/cdp/summary
  *  @param seq The handle to the sequential file structure.
  *  @param v A pointer to the current value to print
  *  @return 0 on success or a negative value on failure.
  */
int cdp_seq_detail_show(struct seq_file *seq, void *v);

#endif
