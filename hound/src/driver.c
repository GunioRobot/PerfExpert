/****************************************************************************
This file is part of Hound.

Hound is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Hound is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Hound.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "err.h"
#include "cpuid.h"
#include "typeDefinitions.h"

#include "discovery/amd.h"
#include "discovery/intel.h"

#ifndef	__x86_64
	#include "nanobenchmarks/x86.h"
#else
	#include "nanobenchmarks/x86-64.h"
#endif

#define	COLLECTION_SIZE	50

char processor = PROC_UNKNOWN;

double get90thPercentile(volatile double* lpCycles, unsigned int count)
{
	int i, j;
	double temp;

	// Sort elements
	for (i=0; i<count-1; i++)
		for (j=i+1; j<count; j++)
			if (lpCycles[i] > lpCycles[j])
				temp=lpCycles[i], lpCycles[i]=lpCycles[j], lpCycles[j]=temp;

	// Return the 90th percentile from the sorted list
	double value = lpCycles[(int) (0.9*count)];
	if (value < 1.0)
		return 1.0;

	return value;
}

int insertIntoCacheList(cacheList** lplpCacheList, cacheInfo cache)
{
	while (*lplpCacheList != NULL)
		lplpCacheList = &((*lplpCacheList)->lpNext);

	if (((*lplpCacheList) = (cacheList*) malloc(sizeof(cacheList))) == NULL)
		return -ERR_MEMFAILED;

	(*lplpCacheList)->info = cache;
	(*lplpCacheList)->lpNext = NULL;
	return 0;
}

void destroyCacheLists(cacheCollection* lpCaches)
{
	cacheList* lpTemp;
	while ((lpTemp = lpCaches->lpL1Caches) != NULL)
	{
		lpCaches->lpL1Caches = lpTemp->lpNext;
		free(lpTemp);
	}

	while ((lpTemp = lpCaches->lpL2Caches) != NULL)
	{
		lpCaches->lpL2Caches = lpTemp->lpNext;
		free(lpTemp);
	}

	while ((lpTemp = lpCaches->lpL2Caches) != NULL)
	{
		lpCaches->lpL2Caches = lpTemp->lpNext;
		free(lpTemp);
	}
}

void discoverCachesWithoutCPUID()
{
	int i, j;
	unsigned int cycles;
	unsigned short counter = 1;
	double cycleCollection [COLLECTION_SIZE];

	#ifdef	DEBUG_PRINT
		printf ("DEBUG: Discovering caches without using CPUID\n");
	#endif

	// First measure with 1KB
	for (j=0; j<COLLECTION_SIZE; j++)
		cycleCollection[j] = allocateAndTest(1024, /* lpCache->info.lineSize */ 512, 1);

	double val;
	val = get90thPercentile(cycleCollection, COLLECTION_SIZE);
	printf ("L1_dlat = %.2lf\n", val);

	// FIXME: L1 Latency values are being calculated incorrectly
	printf ("L1_ilat = %.2lf\n", val);

	// Start measuring from L2
	cycles = val;

	for (i=32768; i<4*1024*1024; i+= 32768)
	{
		for (j=0; j<COLLECTION_SIZE; j++)
			cycleCollection[j] = allocateAndTest(i, /* lpCache->info.lineSize */ 512, 1);

		val = get90thPercentile(cycleCollection, COLLECTION_SIZE);

		if (val > 5*cycles)
		{
			#ifdef	DEBUG_PRINT
				printf ("DEBUG: Found 5x increase in 90th percentile at %d KB\n", i/1024);
			#endif

			if (counter == 1)
				printf ("L%d_dlat = %.2lf\n", ++counter, val);
			else	// Assuming unified caches beyond L1
				printf ("L%d_lat = %.2lf\n", ++counter, val);

			cycles = val;

			if (counter == 3)
			{
				#ifdef	DEBUG_PRINT
					printf ("DEBUG: Discovered L2, stopping probing caches\n");
				#endif

				break;
			}
		}
	}

	#ifdef	DEBUG_PRINT
		printf ("DEBUG: Finished probing caches, now probing memory with size: %.2lf KB\n", 30 * i / 1024.0);
	#endif

	// Start with mem_size = 30i KB
	for (j=0; j<COLLECTION_SIZE; j++)
		cycleCollection[j] = allocateAndTestMainMemory(30 * i, /* lpCache->info.lineSize */ 1024, 0);

	#ifdef	DEBUG_PRINT
		printf ("DEBUG: Finished probing memory\n");
	#endif

	printf ("mem_lat = %.2lf\n", get90thPercentile(cycleCollection, COLLECTION_SIZE));
}

int isCacheProbeSupportedByCPUID(unsigned char processor)
{
	int info[4];

	if (processor == PROC_INTEL)
	{
		__cpuid(info, 0, 0);
		if (info[0] >= 0x2)
			return 0;
	}
	else if (processor == PROC_AMD)
	{
		__cpuid(info, 0x80000000, 0);
		if (info[0] >= 0x80000006)
			return 0;
	}

	return -1;
}

int main( int argc, char * argv[] )
{
	int i, j;
	double cycleCollection [COLLECTION_SIZE];

	#ifdef	DEBUG_PRINT
		printf ("DEBUG: Debug messages ON\n");
	#endif

	if (isCPUIDSupported())
	{
		#ifdef	DEBUG_PRINT
			printf ("DEBUG: CPUID supported, getting processor name...\n");
		#endif

		char processorName[13];
		getProcessorName(processorName);

		if (strcmp(processorName, "GenuineIntel") == 0)
			processor = PROC_INTEL;
		else if (strcmp(processorName, "AuthenticAMD") == 0)
			processor = PROC_AMD;

		#ifdef	DEBUG_PRINT
			printf ("DEBUG: Processor name was: %s\n", processorName);
		#endif

		if (isCacheProbeSupportedByCPUID(processor) < 0)
		{
			#ifdef	DEBUG_PRINT
				printf ("DEBUG: Processor supports CPUID instruction but only limited information can be retrived. Falling back to probing using different memory sizes\n");
			#endif

			// CPUID of no use, instead measure using different memory sizes and latency of access
			processor = PROC_UNKNOWN;
		}
		else
		{
			#ifdef	DEBUG_PRINT
				printf ("DEBUG: Cache probing using CPUID instruction supported\n");
			#endif
		}
	}
	else
	{
		#ifdef	DEBUG_PRINT
			printf ("DEBUG: CPUID instruction not supported\n");
		#endif
	}

	// Version string
	printf ("version = 1.0\n\n");
	printf ("# Generated using hound\n");
	printf ("CPI_threshold = 0.5\n");

	if (processor == PROC_UNKNOWN)
		discoverCachesWithoutCPUID();	// Also prints values
	else
	{
		cacheCollection caches = {0};

		if (processor == PROC_INTEL)	discoverIntelCaches(&caches);
		else if (processor == PROC_AMD)	discoverAMDCaches(&caches);

		#ifdef	DEBUG_PRINT
			printf ("DEBUG: Finished discovering Intel / AMD caches\n");
		#endif

		// Now that we have sizes and characteristics of all caches,
		// measure the latencies to each cache

		// First, L1
		if (caches.lpL1Caches == NULL)
		{
			#ifdef	DEBUG_PRINT
				printf ("DEBUG: No L1 caches found!\n");
			#endif

			printf ("DEBUG: Could not find L1 cache, exiting\n");
			return 1;
		}

		double L1_dlat = 1;
		volatile cacheList* lpCache = caches.lpL1Caches;
		volatile double cycleCollection [COLLECTION_SIZE];
		while (lpCache != NULL)
		{
			if (lpCache->info.cacheOrTLB == CACHE && lpCache->info.type != INSTRUCTION)	// We cannot measure TLB and iCACHE latencies this way, so just measure caches
			{
				#ifdef	DEBUG_PRINT
					if (lpCache->info.cacheOrTLB == CACHE)
						printf ("DEBUG: Measuring latency of L1 %s cache, line size: %d bytes, total size: %.1lf KB\n", getCacheType(lpCache->info.type), lpCache->info.lineSize, lpCache->info.lineSize * lpCache->info.lineCount / 1024.0);
				#endif

				for (i=0; i<COLLECTION_SIZE; i++)
					cycleCollection[i] = allocateAndTest(lpCache->info.lineSize * lpCache->info.lineCount, /* lpCache->info.lineSize */ 512, 1);

				L1_dlat = get90thPercentile(cycleCollection, COLLECTION_SIZE);
				printf ("L1_dlat = %.2lf\n", L1_dlat);
				break;
			}

			lpCache = lpCache->lpNext;
		}

		// FIXME: L1 Latency values are being calculated incorrectly
		// printf ("L1_ilat = %.2lf\n", getIntelL1InstructionLatency());

		// Temporary fix:
		printf ("L1_ilat = %.2lf\n", L1_dlat);

		// Then L2
		lpCache = caches.lpL2Caches;
		while (lpCache != NULL)
		{
			if (lpCache->info.cacheOrTLB == CACHE && lpCache->info.type != INSTRUCTION)	// We cannot measure TLB and iCACHE latencies this way, so just measure caches
			{
				#ifdef	DEBUG_PRINT
					if (lpCache->info.cacheOrTLB == CACHE)
						printf ("DEBUG: Measuring latency of L2 %s cache, line size: %d bytes, total size: %.1lf KB\n", getCacheType(lpCache->info.type), lpCache->info.lineSize, lpCache->info.lineSize * lpCache->info.lineCount / 1024.0);
				#endif

				for (i=0; i<COLLECTION_SIZE; i++)
					cycleCollection[i] = allocateAndTest(lpCache->info.lineSize * lpCache->info.lineCount /2, /* lpCache->info.lineSize */ 512, 1);

				if (lpCache->info.type == UNIFIED)
					printf ("L2_lat = %.2lf\n", get90thPercentile(cycleCollection, COLLECTION_SIZE));
				else
					printf ("L2_dlat = %.2lf\n", get90thPercentile(cycleCollection, COLLECTION_SIZE));
				break;
			}

			lpCache = lpCache->lpNext;
		}

		// L3...
		lpCache = caches.lpL3Caches;
		while (lpCache != NULL)
		{
			#ifdef	DEBUG_PRINT
				if (lpCache->info.cacheOrTLB == CACHE)
					printf ("DEBUG: Measuring latency of L3 %s cache, line size: %d bytes, total size: %.1lf KB\n", getCacheType(lpCache->info.type), lpCache->info.lineSize, lpCache->info.lineSize * lpCache->info.lineCount / 1024.0);
				else
					printf ("DEBUG: Was not expecting anything other than cache, however found an item with code '%d' in the list, ignoring...\n", lpCache->info.cacheOrTLB);
			#endif

			// There are no level 3 TLBs, so not making the extra check
			for (i=0; i<COLLECTION_SIZE; i++)
				cycleCollection[i] = allocateAndTest(lpCache->info.lineSize * lpCache->info.lineCount /2, /* lpCache->info.lineSize */ 512, 1);

			printf ("L3_lat = %.2lf\n", get90thPercentile(cycleCollection, COLLECTION_SIZE));

			// FIXME: Non-sensical to include a `break' here
			break;
			lpCache = lpCache->lpNext;
		}

		// Finally to the main memeory
		if (caches.lpL3Caches)
		{
			lpCache = caches.lpL3Caches;

			#ifdef	DEBUG_PRINT
				printf ("DEBUG: Using memory size as 30x L3 size = %.2lf KB\n", 30 * lpCache->info.lineSize * lpCache->info.lineCount / 1024.0);
			#endif
		}
		else if (caches.lpL2Caches)
		{
			lpCache = caches.lpL2Caches;

			#ifdef	DEBUG_PRINT
				printf ("DEBUG: Using memory size as 30x L2 size = %.2lf KB\n", 30 * lpCache->info.lineSize * lpCache->info.lineCount / 1024.0);
			#endif
		}
		else
		{
			lpCache = caches.lpL1Caches;

			#ifdef	DEBUG_PRINT
				printf ("DEBUG: Using memory size as 30x L1 size = %.2lf KB\n", 30 * lpCache->info.lineSize * lpCache->info.lineCount / 1024.0);
			#endif
		}

		for (i=0; i<COLLECTION_SIZE; i++)
			cycleCollection[i] = allocateAndTestMainMemory(30 * lpCache->info.lineSize * lpCache->info.lineCount, /* lpCache->info.lineSize */ 1024, 0);

		destroyCacheLists(&caches);
		printf ("mem_lat = %.2lf\n", get90thPercentile(cycleCollection, COLLECTION_SIZE));
	}

	// Lookup operating frequency in /proc/cpuinfo
	FILE* fp = fopen("/proc/cpuinfo", "r");
	if (fp == NULL)
	{
		perror ("Error opening /proc/cpuinfo for reading processor frequency");
		return 1;
	}

	char line [256]={0};
	while (fgets(line, 256, fp))
	{
		if (strncmp ("cpu MHz", line, 7) == 0)
			break;
	}

	fclose(fp);
	if (line[0] == '\0')
	{
		printf ("Did not find cpu MHz string in /proc/cpuinfo, is it a valid file?\n");
		return 1;
	}

	double frequency = 0;
	sscanf(line, "cpu MHz :%lf", &frequency);
	printf ("CPU_freq = %.0lf\n", frequency*1000000);

	double FP_lat, FP_slow_lat, BR_lat, BR_miss_lat, TLB_lat;

	FP_lat = getFPLatency();
	printf ("FP_lat = %.2lf\n", FP_lat < 1.0 ? 1.0 : FP_lat);

	FP_slow_lat = getFPSlowLatency();
	printf ("FP_slow_lat = %.2lf\n", FP_slow_lat < 1.0 ? 1.0 : FP_slow_lat);

	BR_lat = getPredictedBranchLatency();
	printf ("BR_lat = %.2lf\n", BR_lat < 1.0 ? 1.0 : BR_lat);

	BR_miss_lat = getMisPredictedBranchLatency();
	printf ("BR_miss_lat = %.2lf\n", BR_miss_lat < 1.0 ? 1.0 : BR_miss_lat);

	TLB_lat = getTLBLatency(512*1024);
	printf ("TLB_lat = %.2lf\n", TLB_lat < 1.0 ? 1.0 : TLB_lat);

	return 0;
}
