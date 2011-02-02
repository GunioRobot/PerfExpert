
#ifndef DISCOVERY_AMD_H_
#define DISCOVERY_AMD_H_

int getAMDL1Wayness(short code)
{
	switch (code)
	{
		case 0:		return UNKNOWN;
		case 1:		return DIRECT_MAPPED;
		case 0xff:	return FULLY_ASSOCIATIVE;
		default:	return code;
	}
}

int getAMDL2Wayness(short code)
{
	switch (code)
	{
		case 1:		return DIRECT_MAPPED;

		case 2:
		case 4:		return code;

		case 6:		return 8;
		case 9:		return 16;
		case 10:	return 32;
		case 11:	return 48;
		case 12:	return 64;
		case 13:	return 96;
		case 14:	return 128;

		case 16:	return FULLY_ASSOCIATIVE;

	/*
		case 0:
		case 3:
		case 5:
		case 7:
		case 9:
	*/

		default:	return UNKNOWN;
	}
}

int getAMDL3Wayness(short code)
{
	if (!code || code > 0xf)
		return UNKNOWN;

	switch(code)
	{
		case 1:		return DIRECT_MAPPED;
		case 2:		return 2;
		case 4:		return 4;
		case 6:		return 8;
		case 8:		return 16;
		case 10:	return 32;
		case 11:	return 48;
		case 12:	return 64;
		case 13:	return 96;
		case 14:	return 128;
		case 15:	return FULLY_ASSOCIATIVE;
	}

	// Should never reach here, but just to eliminate compiler warnings...
	// Oh, and also if code < 0
	return UNKNOWN;
}

int discoverAMDCaches(cacheCollection* lpCaches)
{
	int i, info[4];

	__cpuid(info, 0x80000005);

	// TLBs
	if ((info[EAX] & 0xff000000) >> 24)
	{
		cacheInfo L1DataTLB;
		L1DataTLB.cacheOrTLB = TLB;
		L1DataTLB.type = DATA;
		L1DataTLB.level = 1;

		L1DataTLB.lineSize = PAGESIZE_2M | PAGESIZE_4M;
		L1DataTLB.lineCount = (info[EAX] & 0x00ff0000) >> 16;
		L1DataTLB.wayness = getAMDL1Wayness((info[EAX] & 0xff000000) >> 24);

		#ifdef	DEBUG_PRINT
			printf ("DEBUG: Found L1 data TLB with %d lines\n", L1DataTLB.lineCount);
		#endif

		insertIntoCacheList(&lpCaches->lpL1Caches, L1DataTLB);
	}

	if ((info[EAX] & 0x0000ff00) >> 8)
	{
		cacheInfo L1InstTLB;
		L1InstTLB.cacheOrTLB = TLB;
		L1InstTLB.type = INSTRUCTION;
		L1InstTLB.level = 1;

		L1InstTLB.lineSize = PAGESIZE_2M | PAGESIZE_4M;
		L1InstTLB.lineCount = info[EAX] & 0xff;
		L1InstTLB.wayness = getAMDL1Wayness((info[EAX] & 0x0000ff00) >> 8);

		#ifdef	DEBUG_PRINT
			printf ("DEBUG: Found L1 instruction TLB with %d lines\n", L1InstTLB.lineCount);
		#endif

		insertIntoCacheList(&lpCaches->lpL1Caches, L1InstTLB);
	}

	if ((info[EBX] & 0xff000000) >> 24)
	{
		// TLB for 4 KB pages
		cacheInfo L1DataTLB;
		L1DataTLB.cacheOrTLB = TLB;
		L1DataTLB.type = DATA;
		L1DataTLB.level = 1;

		L1DataTLB.lineSize = PAGESIZE_4K;
		L1DataTLB.lineCount = (info[EBX] & 0x00ff0000) >> 16;
		L1DataTLB.wayness = getAMDL1Wayness((info[EBX] & 0xff000000) >> 24);

		#ifdef	DEBUG_PRINT
			printf ("DEBUG: Found L1 data TLB with %d lines\n", L1DataTLB.lineCount);
		#endif

		insertIntoCacheList(&lpCaches->lpL1Caches, L1DataTLB);
	}

	if ((info[EBX] & 0x0000ff00) >> 8)
	{
		// TLB for 4 KB pages
		cacheInfo L1InstTLB;
		L1InstTLB.cacheOrTLB = TLB;
		L1InstTLB.type = INSTRUCTION;
		L1InstTLB.level = 1;

		L1InstTLB.lineSize = PAGESIZE_4K;
		L1InstTLB.lineCount = info[EBX] & 0xff;
		L1InstTLB.wayness = getAMDL1Wayness((info[EBX] & 0x0000ff00) >> 8);

		#ifdef	DEBUG_PRINT
			printf ("DEBUG: Found L1 instruction TLB with %d lines\n", L1InstTLB.lineCount);
		#endif

		insertIntoCacheList(&lpCaches->lpL1Caches, L1InstTLB);
	}

	if ((info[ECX] & 0x00ff0000) >> 16)
	{
		// Caches
		cacheInfo L1DataCache;
		L1DataCache.cacheOrTLB = CACHE;
		L1DataCache.type = DATA;
		L1DataCache.level = 1;

		L1DataCache.lineSize = info[ECX] & 0xff;
		L1DataCache.lineCount = 1024*((info[ECX] & 0xff000000) >> 24) / L1DataCache.lineSize;
		L1DataCache.wayness = getAMDL1Wayness((info[ECX] & 0x00ff0000) >> 16);

		#ifdef	DEBUG_PRINT
			printf ("DEBUG: Found L1 data cache of size %.2lf KB\n", L1DataCache.lineCount * L1DataCache.lineSize / 1024.0);
		#endif

		insertIntoCacheList(&lpCaches->lpL1Caches, L1DataCache);
	}

	if ((info[EDX] & 0x00ff0000) >> 16)
	{
		cacheInfo L1InstCache;
		L1InstCache.cacheOrTLB = CACHE;
		L1InstCache.type = INSTRUCTION;
		L1InstCache.level = 1;

		L1InstCache.lineSize = info[EDX] & 0xff;
		L1InstCache.lineCount = 1024*((info[EDX] & 0xff000000) >> 24) / L1InstCache.lineSize;
		L1InstCache.wayness = getAMDL1Wayness((info[EDX] & 0x00ff0000) >> 16);

		#ifdef	DEBUG_PRINT
			printf ("DEBUG: Found L1 instruction cache of size %.2lf KB\n", L1InstCache.lineCount * L1InstCache.lineSize / 1024.0);
		#endif

		insertIntoCacheList(&lpCaches->lpL1Caches, L1InstCache);
	}

	__cpuid(info, 0x80000006);

	// TLBs
	if (info[EAX] & 0xf0000000)
	{
		cacheInfo L2DataTLB;
		L2DataTLB.cacheOrTLB = TLB;
		L2DataTLB.type = DATA;
		L2DataTLB.level = 2;

		L2DataTLB.lineSize = PAGESIZE_2M | PAGESIZE_4M;
		L2DataTLB.lineCount = (info[EAX] & 0x0fff0000) >> 16;
		L2DataTLB.wayness = getAMDL3Wayness((info[EAX] & 0xf0000000) >> 28);

		#ifdef	DEBUG_PRINT
			printf ("DEBUG: Found L2 data TLB with %d lines\n", L2DataTLB.lineCount);
		#endif

		insertIntoCacheList(&lpCaches->lpL2Caches, L2DataTLB);
	}

	if (info[EAX] & 0x0000f000)
	{
		cacheInfo L2InstTLB;
		L2InstTLB.cacheOrTLB = TLB;
		L2InstTLB.type = INSTRUCTION;
		L2InstTLB.level = 2;

		L2InstTLB.lineSize = PAGESIZE_2M | PAGESIZE_4M;
		L2InstTLB.lineCount = info[EAX] & 0x00000fff;
		L2InstTLB.wayness = getAMDL3Wayness((info[EAX] & 0x0000f000) >> 12);

		#ifdef	DEBUG_PRINT
			printf ("DEBUG: Found L2 instruction TLB with %d lines\n", L2InstTLB.lineCount);
		#endif

		insertIntoCacheList(&lpCaches->lpL2Caches, L2InstTLB);
	}

	// 4 KB TLBs
	if ((info[EBX] & 0xf0000000) >> 28)
	{
		cacheInfo L2DataTLB;
		L2DataTLB.cacheOrTLB = TLB;
		L2DataTLB.type = DATA;
		L2DataTLB.level = 2;

		L2DataTLB.lineSize = PAGESIZE_4K;
		L2DataTLB.lineCount = (info[EBX] & 0x0fff0000) >> 16;
		L2DataTLB.wayness = getAMDL3Wayness((info[EBX] & 0xf0000000) >> 28);

		#ifdef	DEBUG_PRINT
			printf ("DEBUG: Found L2 data TLB with %d lines\n", L2DataTLB.lineCount);
		#endif

		insertIntoCacheList(&lpCaches->lpL2Caches, L2DataTLB);
	}

	if (info[EBX] & 0x0000f000)
	{
		cacheInfo L2InstTLB;
		L2InstTLB.cacheOrTLB = TLB;
		L2InstTLB.type = INSTRUCTION;
		L2InstTLB.level = 2;

		L2InstTLB.lineSize = PAGESIZE_4K;
		L2InstTLB.lineCount = info[EBX] & 0x00000fff;
		L2InstTLB.wayness = getAMDL3Wayness((info[EBX] & 0x0000f000) >> 12);

		#ifdef	DEBUG_PRINT
			printf ("DEBUG: Found L2 instruction TLB with %d lines\n", L2InstTLB.lineCount);
		#endif

		insertIntoCacheList(&lpCaches->lpL2Caches, L2InstTLB);
	}

	// Cache
	if (info[ECX] & 0x0000f000)     // Not disabled
	{
		cacheInfo L2Cache;
		L2Cache.cacheOrTLB = CACHE;
		L2Cache.type = UNIFIED;
		L2Cache.level = 2;

		L2Cache.lineSize = info[ECX] & 0xff;
		L2Cache.lineCount = 1024*((info[ECX] & 0xffff0000) >> 16) / L2Cache.lineSize;
		L2Cache.wayness = getAMDL2Wayness((info[ECX] & 0x0000f000) >> 12);

		#ifdef	DEBUG_PRINT
			printf ("DEBUG: Found L2 unified cache of size %.2lf KB\n", L2Cache.lineCount * L2Cache.lineSize / 1024.0);
		#endif

		insertIntoCacheList(&lpCaches->lpL2Caches, L2Cache);
	}

	if (info[EDX] & 0x0000f000)     // Not disabled
	{
		cacheInfo L3Cache;
		L3Cache.cacheOrTLB = CACHE;
		L3Cache.type = UNIFIED;
		L3Cache.level = 3;

		L3Cache.lineSize = info[EDX] & 0xff;
		L3Cache.lineCount = 1024*512*((info[EDX] & 0xfffc0000) >> 18) / L3Cache.lineSize;
		L3Cache.wayness = getAMDL3Wayness((info[EDX] & 0x0000f000) >> 12);

		#ifdef	DEBUG_PRINT
			printf ("DEBUG: Found L3 unified cache of size %.2lf KB\n", L3Cache.lineCount * L3Cache.lineSize / 1024.0);
		#endif

		insertIntoCacheList(&lpCaches->lpL3Caches, L3Cache);
	}
}

#endif /* DISCOVERY_AMD_H_ */
