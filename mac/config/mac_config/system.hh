/*
	system.hh
	---------
*/

#ifndef MACCONFIG_SYSTEM_HH
#define MACCONFIG_SYSTEM_HH


#define MAC_CONFIG_SYSTEM_6        600
#define MAC_CONFIG_SYSTEM_6_0_4    604

#define MAC_CONFIG_SYSTEM_7        700
#define MAC_CONFIG_SYSTEM_7_1      710
#define MAC_CONFIG_SYSTEM_7_1_2    712
#define MAC_CONFIG_SYSTEM_7_5      750
#define MAC_CONFIG_SYSTEM_7_6      760

#define MAC_CONFIG_MAC_OS_8        800
#define MAC_CONFIG_MAC_OS_8_1      810
#define MAC_CONFIG_MAC_OS_8_5      850
#define MAC_CONFIG_MAC_OS_8_6      860

#define MAC_CONFIG_MAC_OS_9        900
#define MAC_CONFIG_MAC_OS_9_1      910
#define MAC_CONFIG_MAC_OS_9_2      920

#define MAC_CONFIG_MAC_OS_X       1000
#define MAC_CONFIG_MACOSX_10_0    1000
#define MAC_CONFIG_MACOSX_10_1    1010
#define MAC_CONFIG_MACOSX_10_2    1020
#define MAC_CONFIG_MACOSX_10_3    1030
#define MAC_CONFIG_MACOSX_10_4    1040
#define MAC_CONFIG_MACOSX_10_5    1050
#define MAC_CONFIG_MACOSX_10_6    1060
#define MAC_CONFIG_MACOSX_10_7    1070

#define MAC_CONFIG_LATEST_SYSTEM  9999

#ifndef CONFIG_MINIMUM_SYSTEM
#ifdef __MC68K__
	/*
		System 6.0.4 introduced Gestalt().  We can downgrade this requirement
		if we check for _Gestalt before calling it.
	*/
	#define CONFIG_MINIMUM_SYSTEM  MAC_CONFIG_SYSTEM_6_0_4
#elif ! TARGET_API_MAC_CARBON
	/*
		System 7.1.2 is the first system supporting PPC machines.
		Upgrading this requirement to System 7.5 wouldn't be unreasonable.
	*/
	#define CONFIG_MINIMUM_SYSTEM  MAC_CONFIG_SYSTEM_7_1_2
#elif ! defined( __APPLE__ )
	/*
		Carbon definitely requires at least Mac OS 8.  Later versions offer
		better support, though, so an upgrade to 8.5, 9, or even X wouldn't be
		out of the question for some applications.
	*/
	#define CONFIG_MINIMUM_SYSTEM  MAC_CONFIG_MAC_OS_8
#elif ! defined( __LITTLE_ENDIAN__ )
	/*
		Programs built with Apple's developer tools obviously require Mac OS X.
		Realistically, this can be set higher.  Apple's own headers set 10.1
		as the minimum for PPC.  The first version of Mac OS X even remotely
		usable was 10.2, but developer tools for it don't seem to be available
		anymore.  So 10.3 is a reasonable value, and even 10.4 has its merits,
		since any machine too old to run 10.4 would be much happier with OS 9.
	*/
	#define CONFIG_MINIMUM_SYSTEM  MAC_CONFIG_MAC_OS_X
#else
	/*
		Mac OS X begins supporting little endian in version 10.4.
	*/
	#define CONFIG_MINIMUM_SYSTEM  MAC_CONFIG_MACOSX_10_4
#endif
#endif

#ifndef CONFIG_MAXIMUM_SYSTEM
#if ! TARGET_API_MAC_CARBON
	/*
		68K code and non-Carbon PPC code both work up through Mac OS 9.
	*/
	#define CONFIG_MAXIMUM_SYSTEM  (MAC_CONFIG_MAC_OS_X - 1)
#elif ! defined( __LITTLE_ENDIAN__ )
	/*
		CFM and Mach-O PPC code are supported via Rosetta emulation up through
		Mac OS X 10.6.
	*/
	#define CONFIG_MAXIMUM_SYSTEM  (MAC_CONFIG_MACOSX_10_7 - 1)
#else
	#define CONFIG_MAXIMUM_SYSTEM  MAC_CONFIG_LATEST_SYSTEM
#endif
#endif

#if CONFIG_MINIMUM_SYSTEM > CONFIG_MAXIMUM_SYSTEM
#error CONFIG_MINIMUM_SYSTEM must not exceed CONFIG_MAXIMUM_SYSTEM
#endif

#define MAC_CONFIG_REQUIRES( version )  (CONFIG_MINIMUM_SYSTEM >= version)
#define MAC_CONFIG_FEATURES( version )  (CONFIG_MAXIMUM_SYSTEM >= version)

#define CONFIG_SYSTEM_7_REQUIRED    MAC_CONFIG_REQUIRES( MAC_CONFIG_SYSTEM_7   )
#define CONFIG_SYSTEM_7_1_REQUIRED  MAC_CONFIG_REQUIRES( MAC_CONFIG_SYSTEM_7_1 )
#define CONFIG_SYSTEM_7_5_REQUIRED  MAC_CONFIG_REQUIRES( MAC_CONFIG_SYSTEM_7_5 )
#define CONFIG_SYSTEM_7_6_REQUIRED  MAC_CONFIG_REQUIRES( MAC_CONFIG_SYSTEM_7_6 )

#define CONFIG_SYSTEM_7_FEATURED    MAC_CONFIG_FEATURES( MAC_CONFIG_SYSTEM_7   )
#define CONFIG_SYSTEM_7_1_FEATURED  MAC_CONFIG_FEATURES( MAC_CONFIG_SYSTEM_7_1 )
#define CONFIG_SYSTEM_7_5_FEATURED  MAC_CONFIG_FEATURES( MAC_CONFIG_SYSTEM_7_5 )
#define CONFIG_SYSTEM_7_6_FEATURED  MAC_CONFIG_FEATURES( MAC_CONFIG_SYSTEM_7_6 )

#endif
