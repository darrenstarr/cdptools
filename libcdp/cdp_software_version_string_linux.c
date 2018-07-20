#include "cdp_software_version_string.h"
#include "platform/platform.h"
#include "platform/string.h"
#include "platform/types.h"
#include "platform/utsname.h"

const char *cdp_platform_string = "Linux";

int generate_cdp_software_version_string(char **result)
{
	UTS_STRUCT system_information;
	ssize_t buffer_size_required;

	static const char software_version_format[] =
		"Linux\nhttps://www.kernel.org/\n%s\n%s\n";

	char work_buffer[
		sizeof(software_version_format) +
			UTS_VERSION_LENGTH +
			UTS_RELEASE_LENGTH +
			1
	];

	if (*result != NULL)
	{
		LOG_CRITICAL("generate_cdp_software_version_string: the string has already been allocated. aborting\n");
		return -1;
	}

	GET_UTSNAME_IF(system_information)
	{
		LOG_CRITICAL("generate_cdp_software_version_string: failed to obtain system information\n");
		return -1;
	}

	buffer_size_required =
		snprintf(
			work_buffer,
			sizeof(work_buffer),
			software_version_format,
			UTS_VERSION(system_information),
			UTS_RELEASE(system_information)
		);

	if (buffer_size_required < 0)
	{
		LOG_CRITICAL("generate_cdp_software_version_string: failed to calculate the buffer size required.\n");
		return -1;
	}

	*result = (char *)ALLOC_NEW_ARRAY(char, (size_t)(buffer_size_required + 1));
	if (*result == NULL)
	{
		LOG_CRITICAL("generate_cdp_software_version_string: failed to allocate buffer.\n");
		return -1;
	}

	strcpy(*result, work_buffer);

	return 0;
}

int generate_cdp_device_id_string(char **result)
{
	UTS_STRUCT system_information;
	ssize_t buffer_size_required;

	if (*result != NULL)
	{
		LOG_CRITICAL("generate_cdp_device_id_string: the string has already been allocated. aborting\n");
		return -1;
	}

	GET_UTSNAME_IF(system_information)
	{
		LOG_CRITICAL("generate_cdp_device_id_string: failed to obtain system information\n");
		return -1;
	}

	buffer_size_required =
		(ssize_t)strlen(UTS_NODE_NAME(system_information)) +
		(
			(UTS_DOMAIN_NAME(system_information)[0] == 0) ?
				1 :
				(ssize_t)(strlen(UTS_DOMAIN_NAME(system_information)) + 2)
		)
		;

	if (buffer_size_required < 0)
	{
		LOG_CRITICAL("generate_cdp_device_id_string: failed to calculate the buffer size required.\n");
		return -1;
	}

	*result = (char *)ALLOC_NEW_ARRAY(char, (size_t)(buffer_size_required + 1));
	if (*result == NULL)
	{
		LOG_CRITICAL("generate_cdp_software_version_string: failed to allocate buffer.\n");
		return -1;
	}

	strcpy(*result, UTS_NODE_NAME(system_information));
	if (UTS_DOMAIN_NAME(system_information)[0] != 0)
	{
		strcat(*result, ".");
		strcat(*result, UTS_DOMAIN_NAME(system_information));
	}

	return 0;
}
