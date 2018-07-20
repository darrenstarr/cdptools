#ifndef CDP_SOFTWARE_VERSION_STRING_H
#define CDP_SOFTWARE_VERSION_STRING_H

/** This is the platform ID string that is send to all CDP neightbors to describe this device */
extern const char *cdp_platform_string;

/** Use the system information to produce a software version string to pass as part of CDP.
  *  @param result A pointer to a char * to allocate and store the result in.
  *  @return 0 on success, a negative value on error.
  */
int generate_cdp_software_version_string(char **result);

/** Use the system information to produce a device ID string to pass as part of CDP.
  *  @param result A point to a char pointer to allocate and return the result to.
  *  @return 0 on success, a negative value on error.
  */
int generate_cdp_device_id_string(char **result);

#endif
