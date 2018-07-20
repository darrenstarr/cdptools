#ifndef ECDPTLV_H
#define ECDPTLV_H

typedef enum
{
	CdpTlvDeviceId = 1,
	CdpTlvAddresses = 2,
	CdpTlvPortId = 3,
	CdpTlvCapabilities = 4,
	CdpTlvSoftwareVersion = 5,
	CdpTlvPlatform = 6,
	CdpTlvODRPrefixes = 7,
	CdpTlvClusterManagementProtocol = 8,
	CdpTlvVtpManagementDomain = 9,
	CdpTlvNativeVlan = 10,
	CdpTlvDuplex = 11,
	CdpTlvTrustBitmap = 18,
	CdpTlvUntrustedPortCoS = 19,
	CdpTlvManagementAddesses = 22,
	CdpTlvPowerAvailable = 26,
	CdpTlvStartupNativeVlan = 0x1007
} ECdpTlv;

#endif
