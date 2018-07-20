#ifdef _MSC_VER

#include "cdp_software_version_string.h"
#include "platform/platform.h"
#include "platform/string.h"
#include "platform/types.h"
#include <windows.h>
#include <tchar.h>

const char *cdp_platform_string = "Windows10";

/* TODO: Consider making the get version information stuff either Unicode or ensure that it returns only English in any region. */
int generate_cdp_software_version_string(char **result)
{
	ssize_t buffer_size_required;
	OSVERSIONINFOA osvi;

	static const char software_version_format[] =
		"Windows\n"
		"https://www.microsoft.com/en-us/windows\n"
		"Version - %d.%d\n"
		"Build - %d\n"
		"Service pack - %s\n"
		"\n"
		;

	char work_buffer[
		sizeof(software_version_format) +
			12 + 12 +						/* Major version (up to 12 digits) + Minor Version (up to 12 digits) */
			12 +							/* Build Number (up to 12 digits) */
			128 +							/* Service pack information (up to 128 characters) */
			1
	];

	if (*result != NULL)
	{
		LOG_CRITICAL("generate_cdp_software_version_string: the string has already been allocated. aborting\n");
		return -1;
	}

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOA));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

	/* Getting the Windows version information for the purpose of deciding whether code will run or not is considered deprecated in Windows now.
	 * The problem is that this is a tool that is acquiring the version information for informational purposes only. Therefore, I'm using
	 * this function in hopes it won't go away any time soon.
	 */
#pragma warning(push)
#pragma warning(disable: 4996)	// Deprecated warning
	if(!GetVersionExA(&osvi))
	{
		_tprintf(TEXT("GetComputerNameEx failed (%d)\n"), GetLastError());
		LOG_CRITICAL("generate_cdp_software_version_string: failed to obtain system information\n");
		return -1;
	}
#pragma warning(pop)

	buffer_size_required =
		snprintf(
			work_buffer,
			sizeof(work_buffer),
			software_version_format,
			osvi.dwMajorVersion,
			osvi.dwMinorVersion,
			osvi.dwBuildNumber,
			osvi.szCSDVersion
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
	char buffer[256] = "";
	DWORD buffer_size_required = sizeof(buffer) / sizeof(buffer[0]);

	if (*result != NULL)
	{
		LOG_CRITICAL("generate_cdp_device_id_string: the string has already been allocated. aborting\n");
		return -1;
	}

	if (!GetComputerNameExA(ComputerNameDnsFullyQualified, buffer, &buffer_size_required))
	{
		LOG_CRITICAL("generate_cdp_device_id_string: unable to get FQDN of system from the Windows API\n");
		return -1;
	}

	*result = (char *)ALLOC_NEW_ARRAY(char, (size_t)(buffer_size_required + 1));
	if (*result == NULL)
	{
		LOG_CRITICAL("generate_cdp_software_version_string: failed to allocate buffer.\n");
		return -1;
	}

	strcpy(*result, buffer);

	return 0;
}

#endif
