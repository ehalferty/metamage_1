/*
	setpshared.h
	------------
*/

#ifndef CONFIG_SETPSHARED_H
#define CONFIG_SETPSHARED_H


#ifndef CONFIG_SETPSHARED
	#ifdef __RELIX__
		#define CONFIG_SETPSHARED 0
	#endif
	
	#ifdef __OpenBSD__
		#define CONFIG_SETPSHARED 0
	#endif
	
	#ifndef CONFIG_SETPSHARED
		#define CONFIG_SETPSHARED 1
	#endif
#endif


#endif