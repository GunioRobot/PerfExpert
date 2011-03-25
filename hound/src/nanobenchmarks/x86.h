
#ifndef GENERIC_H_
#define GENERIC_H_

#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

double allocateAndTestMainMemory(long size, int stride, unsigned char warmup)
{
	if (stride > size)
		return 0;

	int i, ret, diff, count;
	unsigned char *ptr;

	if ((ret = posix_memalign((void**) &ptr, 8, size) != 0) || ptr == NULL)
	{
		if (errno == ENOMEM)
			printf ("No memory\n");
		else if (errno == EINVAL)
			printf ("Not aligned\n");
		else	printf ("Unknown error in allocating %ld size with %ld alignment\n", size, size);

		return 0;
	}

	long elements = size/stride-1;	// Assuming completely divisible
	int* swapBuffer = malloc(sizeof(int)*elements);
	unsigned int* mem = (unsigned int *) ptr;
	if (swapBuffer == NULL)
	{
		// Try the usual +512 skip
		for (i=0; i<elements-1; i++)
		{
			*mem = (unsigned int) &ptr [(i+1)*stride];
			mem = (unsigned int*) *mem;
		}

		// Loop back
		*mem = (unsigned int) &ptr[0];
	}
	else
	{
		srand(time(NULL));
		int randomNumber;
		for (i=0; i<elements; i++)
			swapBuffer[i] = i;

		// > 0 is on purpose, should not generate zero as one of the random numbers
		for (i=elements-1; i>0; i--)
		{
			int temp = i*((float) rand() / RAND_MAX);
			randomNumber = swapBuffer [temp];
			swapBuffer[temp] = swapBuffer[i];
			*mem = (unsigned int) &(ptr[randomNumber*512]);
			mem = (unsigned int*) *mem;
		}

		// Loop back to the first location
		*mem = (unsigned int) ptr;

		free(swapBuffer);
	}

	int repeatCount = (warmup == 0 ? 1 : 2);

repeatIfNeg:
	__asm__ volatile(
		// Simulate a function call during entry
		"pushl	%2\n\t"		// 16(%%ebp)
		"pushl	%3\n\t"		// 12(%%ebp)
		"pushl	%4\n\t"
		"pushl	%5\n\t"

		// Set up the frame
		"pushl	%%ebp\n\t"
		"movl	%%esp, %%ebp\n\t"
		"subl	$20, %%esp\n\t"

		// Push all regs that we will use in this routine, which is basically all GPRs plus a few others
		"pushl	%%ebx\n\t"
		"pushl	%%esi\n\t"
		"pushl	%%edi\n\t"

		// Setting up repeat counter
		"movl	4(%%ebp), %%ebx\n\t"
		"movl	%%ebx, -4(%%ebp)\n\t"

	/*
		"xorl	%%ebx, %%ebx\n\t"
		"movl	16(%%ebp), %%edx\n\t"
		"movl	12(%%ebp), %%ecx\n\t"
		"movl	8(%%ebp), %%esi\n\t"
		"movl	%%edx, %%edi\n\t"
		"addl	%%esi, %%edi\n\t"

	"setupMM:\n\t"
		"cmpl	$0, %%ecx\n\t"
		"je	setupoutMM\n\t"
		"movl	%%edi, (%%edx, %%ebx)\n\t"
		"subl	%%esi, %%ecx\n\t"
		"addl	%%esi, %%ebx\n\t"
		"addl	%%esi, %%edi\n\t"
		"jmp	setupMM\n\t"

	"setupoutMM:\n\t"
		// "Cyclic reference, last location contains address of first location
		"movl	%%edx, (%%edx, %%ebx)\n\t"
	*/

	/*
		// Change the count of times we loop over this cyclic array
		"movl	12(%%ebp), %%ecx\n\t"
		"addl	%%esi, %%ecx\n\t"

	"shift:\n\t"
		"cmpl	$0, %%esi\n\t"
		"je	shiftout\n\t"
		"shrl	$1, %%esi\n\t"
		"shrl	$1, %%ecx\n\t"
		"jmp	shift\n\t"

	"shiftout:\n\t"
	*/
		// Loop a million times
		"movl	$1048576, %%ecx\n\t"
		"movl	%%ecx, 12(%%ebp)\n\t"

	"repeatMM:\n\t"
		// Start measuring
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"movl	%%eax, -8(%%ebp)\n\t"
		"movl	%%edx, -12(%%ebp)\n\t"

		// Do some operation
		"xorl	%%ebx, %%ebx\n\t"
		"movl	16(%%ebp), %%edx\n\t"
		"movl	12(%%ebp), %%ecx\n\t"
		"movl	8(%%ebp), %%esi\n\t"

	"writeMM:\n\t"
		"cmpl	$0, %%ecx\n\t"
		"je	outMM\n\t"
		"movl	(%%edx), %%edi\n\t"
		"movl	%%edi, %%edx\n\t"
		"decl	%%ecx\n\t"
		"addl	%%esi, %%ebx\n\t"
		"jmp	writeMM\n\t"

	"outMM:\n\t"
		// End measuring
		"movl	%%ebx, -16(%%ebp)\n\t"
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"subl	-8(%%ebp), %%eax\n\t"
		"sbb	-12(%%ebp), %%edx\n\t"

		// Check if we have to repeat
		"decl	-4(%%ebp)\n\t"
		"jnz	repeatMM\n\t"

		// Store the result (with overhead)
		"movl	%%eax, -20(%%ebp)\n\t"

		// Do everything all over again without the actual step to be measured
		// Start measuring
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"movl	%%eax, -8(%%ebp)\n\t"
		"movl	%%edx, -12(%%ebp)\n\t"

		// Do some operation
		"xorl	%%ebx, %%ebx\n\t"
		"movl	16(%%ebp), %%edx\n\t"
		"movl	12(%%ebp), %%ecx\n\t"
		"movl	8(%%ebp), %%esi\n\t"

	"overheadwriteMM:\n\t"
		"cmpl	$0, %%ecx\n\t"
		"je	overheadoutMM\n\t"
		"movl	%%edi, %%edx\n\t"
		"decl	%%ecx\n\t"
		"addl	%%esi, %%ebx\n\t"
		"jmp	overheadwriteMM\n\t"

	"overheadoutMM:\n\t"
		// End measuring
		"movl	%%ebx, -16(%%ebp)\n\t"
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"subl	-8(%%ebp), %%eax\n\t"
		"sbb	-12(%%ebp), %%edx\n\t"

		// Subtract the overhead
		"movl	-20(%%ebp), %%ebx\n\t"
		"subl	%%eax, %%ebx\n\t"
		"movl	%%ebx, %%eax\n\t"

		// Store the results
		"movl	%%eax, %0\n\t"
		"movl	-16(%%ebp), %1\n\t"

		// End of routine, pop all that we had pushed
		"popl	%%edi\n\t"
		"popl	%%esi\n\t"
		"popl	%%ebx\n\t"

		"addl	$20, %%esp\n\t"
		"popl	%%ebp\n\t"

		// Remove the initial user parameters
		"addl	$16, %%esp\n\t"
	: "=a" (diff), "=c" (count)
	: "m" (ptr), "m" (size), "m" (stride), "m" (repeatCount)
	: "esi"
	);

	if (((long) diff) < 0)
		goto repeatIfNeg;

	free(ptr);
	// printf ("Count = %d, diff = %d\n", count/stride, diff);

	return diff / ((float) (count / ((float) stride)));
}

double allocateAndTest(long size, int stride, unsigned char warmup)
{
	if (stride > size)
		return 0;

	int ret, diff, count;
	unsigned char *ptr;

	if ((ret = posix_memalign((void**) &ptr, 8, size) != 0) || ptr == NULL)
	{
		if (errno == ENOMEM)
			printf ("No memory\n");
		else if (errno == EINVAL)
			printf ("Not aligned\n");
		else	printf ("Unknown error in allocating %ld size with %ld alignment\n", size, size);

		return 0;
	}

	int repeatCount = (warmup == 0 ? 1 : 2);

repeatIfNeg:
	__asm__ volatile(
		// Simulate a function call during entry
		"pushl	%2\n\t"		// 16(%%ebp)
		"pushl	%3\n\t"		// 12(%%ebp)
		"pushl	%4\n\t"
		"pushl	%5\n\t"

		// Set up the frame
		"pushl	%%ebp\n\t"
		"movl	%%esp, %%ebp\n\t"
		"subl	$20, %%esp\n\t"

		// Push all regs that we will use in this routine, which is basically all GPRs plus a few others
		"pushl	%%ebx\n\t"
		"pushl	%%esi\n\t"
		"pushl	%%edi\n\t"

		// Setting up repeat counter
		"movl	4(%%ebp), %%ebx\n\t"
		"movl	%%ebx, -4(%%ebp)\n\t"

		"xorl	%%ebx, %%ebx\n\t"
		"movl	16(%%ebp), %%edx\n\t"
		"movl	12(%%ebp), %%ecx\n\t"
		"movl	8(%%ebp), %%esi\n\t"
		"movl	%%edx, %%edi\n\t"
		"addl	%%esi, %%edi\n\t"

	"setup:\n\t"
		"cmpl	$0, %%ecx\n\t"
		"je	setupout\n\t"
		"movl	%%edi, (%%edx, %%ebx)\n\t"
		"subl	%%esi, %%ecx\n\t"
		"addl	%%esi, %%ebx\n\t"
		"addl	%%esi, %%edi\n\t"
		"jmp	setup\n\t"

	"setupout:\n\t"
		// "Cyclic reference, last location contains address of first location
		"movl	%%edx, (%%edx, %%ebx)\n\t"

	/*
		// Change the count of times we loop over this cyclic array
		"movl	12(%%ebp), %%ecx\n\t"
		"addl	%%esi, %%ecx\n\t"

	"shift:\n\t"
		"cmpl	$0, %%esi\n\t"
		"je	shiftout\n\t"
		"shrl	$1, %%esi\n\t"
		"shrl	$1, %%ecx\n\t"
		"jmp	shift\n\t"

	"shiftout:\n\t"
	*/
		// Loop a million times
		"movl	$1048576, %%ecx\n\t"
		"movl	%%ecx, 12(%%ebp)\n\t"

	"repeat:\n\t"
		// Start measuring
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"movl	%%eax, -8(%%ebp)\n\t"
		"movl	%%edx, -12(%%ebp)\n\t"

		// Do some operation
		"xorl	%%ebx, %%ebx\n\t"
		"movl	16(%%ebp), %%edx\n\t"
		"movl	12(%%ebp), %%ecx\n\t"
		"movl	8(%%ebp), %%esi\n\t"

	"write:\n\t"
		"cmpl	$0, %%ecx\n\t"
		"je	out\n\t"
		"movl	(%%edx), %%edi\n\t"
		"movl	%%edi, %%edx\n\t"
		//"subl	%%esi, %%ecx\n\t"
		"decl	%%ecx\n\t"
		"addl	%%esi, %%ebx\n\t"
		"jmp	write\n\t"

	"out:\n\t"
		// End measuring
		"movl	%%ebx, -16(%%ebp)\n\t"
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"subl	-8(%%ebp), %%eax\n\t"
		"sbb	-12(%%ebp), %%edx\n\t"

		// Check if we have to repeat
		"decl	-4(%%ebp)\n\t"
		"jnz	repeat\n\t"

		// Store the result (with overhead)
		"movl	%%eax, -20(%%ebp)\n\t"

		// Do everything all over again without the actual step to be measured
		// Start measuring
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"movl	%%eax, -8(%%ebp)\n\t"
		"movl	%%edx, -12(%%ebp)\n\t"

		// Do some operation
		"xorl	%%ebx, %%ebx\n\t"
		"movl	16(%%ebp), %%edx\n\t"
		"movl	12(%%ebp), %%ecx\n\t"
		"movl	8(%%ebp), %%esi\n\t"

	"overheadwrite:\n\t"
		"cmpl	$0, %%ecx\n\t"
		"je	overheadout\n\t"
		"movl	%%edi, %%edx\n\t"
		//"subl	%%esi, %%ecx\n\t"
		"decl	%%ecx\n\t"
		"addl	%%esi, %%ebx\n\t"
		"jmp	overheadwrite\n\t"

	"overheadout:\n\t"
		// End measuring
		"movl	%%ebx, -16(%%ebp)\n\t"
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"subl	-8(%%ebp), %%eax\n\t"
		"sbb	-12(%%ebp), %%edx\n\t"

		// Subtract the overhead
		"movl	-20(%%ebp), %%ebx\n\t"
		"subl	%%eax, %%ebx\n\t"
		"movl	%%ebx, %%eax\n\t"

		// Store the results
		"movl	%%eax, %0\n\t"
		"movl	-16(%%ebp), %1\n\t"

		// End of routine, pop all that we had pushed
		"popl	%%edi\n\t"
		"popl	%%esi\n\t"
		"popl	%%ebx\n\t"

		"addl	$20, %%esp\n\t"
		"popl	%%ebp\n\t"

		// Remove the initial user parameters
		"addl	$16, %%esp\n\t"
	: "=a" (diff), "=c" (count)
	: "m" (ptr), "m" (size), "m" (stride), "m" (repeatCount)
	: "esi"
	);

	if (((long) diff) < 0)
		goto repeatIfNeg;

	free(ptr);
	// printf ("Count = %d, diff = %d\n", count/stride, diff);

	return diff / ((float) (count / ((float) stride)));
}

double getFPLatency()
{
	int diff, count;
	float op1 = 1424.4525;
	float op2 = 0.5636;

	__asm__ volatile(
		// Simulate a function call during entry
		"pushl	%2\n\t"		// 8(%%ebp)
		"pushl	%3\n\t"		// 4(%%ebp)

		// Set up the frame
		"pushl	%%ebp\n\t"
		"movl	%%esp, %%ebp\n\t"
		"subl	$20, %%esp\n\t"

		// Push all regs that we will use in this routine, which is basically all GPRs plus a few others
		"pushl	%%ebx\n\t"
		"pushl	%%esi\n\t"
		"pushl	%%edi\n\t"

		// Start measuring
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"movl	%%eax, -8(%%ebp)\n\t"
		"movl	%%edx, -12(%%ebp)\n\t"

		// Set up the loop
		"movl	$65536, %%ecx\n\t"
		"xorl	%%ebx, %%ebx\n\t"
		"fld	8(%%ebp)\n\t"

	"fploop:\n\t"
		"cmp $0, %%ecx\n\t"
		"je	fpout\n\t"
		"fadd	8(%%ebp)\n\t"
		"fmul	4(%%ebp)\n\t"
		"fsub	8(%%ebp)\n\t"
		"decl	%%ecx\n\t"
		"incl	%%ebx\n\t"
		"jmp	fploop\n\t"

		// End measuring
	"fpout:\n\t"
		"movl	%%ebx, -16(%%ebp)\n\t"
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"subl	-8(%%ebp), %%eax\n\t"
		"sbb	-12(%%ebp), %%edx\n\t"
		"movl	-16(%%ebp), %%ebx\n\t"

		// Store the result (with overhead)
		"movl	%%eax, -20(%%ebp)\n\t"

		// Do everything all over again without the actual step to be measured
		// Start measuring
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"movl	%%eax, -8(%%ebp)\n\t"
		"movl	%%edx, -12(%%ebp)\n\t"

		// Set up the loop
		"movl	$65536, %%ecx\n\t"
		"xorl	%%ebx, %%ebx\n\t"
		"fld	8(%%ebp)\n\t"

	"overheadfploop:\n\t"
		"cmp $0, %%ecx\n\t"
		"je	overheadfpout\n\t"
		"decl	%%ecx\n\t"
		"incl	%%ebx\n\t"
		"jmp	overheadfploop\n\t"

		// End measuring
	"overheadfpout:\n\t"
		"movl	%%ebx, -16(%%ebp)\n\t"
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"subl	-8(%%ebp), %%eax\n\t"
		"sbb	-12(%%ebp), %%edx\n\t"

		// Subtract the overhead
		"movl	-20(%%ebp), %%ebx\n\t"
		"subl	%%eax, %%ebx\n\t"
		"movl	%%ebx, %%eax\n\t"
		"movl	-16(%%ebp), %%ebx\n\t"

		// Store the results
		"movl	%%eax, %0\n\t"
		"movl	%%ebx, %1\n\t"

		// End of routine, pop all that we had pushed
		"popl	%%edi\n\t"
		"popl	%%esi\n\t"
		"popl	%%ebx\n\t"

		"addl	$20, %%esp\n\t"
		"popl	%%ebp\n\t"

		// Remove the initial user parameters
		"addl	$8, %%esp\n\t"
	: "=a" (diff), "=c" (count)
	: "m" (op1), "m" (op2)
	: "edx", "esi"
	);

	// printf ("Count = %d, diff = %d\n", count, diff);
	return diff / ((double) count*3);
}

double getFPSlowLatency()
{
	int diff, count;
	float op1 = 1424.4525;
	float op2 = 0.5636;

	__asm__ volatile(
		// Simulate a function call during entry
		"pushl	%2\n\t"		// 8(%%ebp)
		"pushl	%3\n\t"		// 4(%%ebp)

		// Set up the frame
		"pushl	%%ebp\n\t"
		"movl	%%esp, %%ebp\n\t"
		"subl	$20, %%esp\n\t"

		// Push all regs that we will use in this routine, which is basically all GPRs plus a few others
		"pushl	%%ebx\n\t"
		"pushl	%%esi\n\t"
		"pushl	%%edi\n\t"

		// Start measuring
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"movl	%%eax, -8(%%ebp)\n\t"
		"movl	%%edx, -12(%%ebp)\n\t"

		// Set up the loop
		"movl	$65536, %%ecx\n\t"
		"xorl	%%ebx, %%ebx\n\t"
		"fld	8(%%ebp)\n\t"

	"fpslowloop:\n\t"
		"cmp $0, %%ecx\n\t"
		"je	fpslowout\n\t"
		"fdiv	4(%%ebp)\n\t"
		"fsqrt\n\t"
		"decl	%%ecx\n\t"
		"incl	%%ebx\n\t"
		"jmp	fpslowloop\n\t"

		// End measuring
	"fpslowout:\n\t"
		"movl	%%ebx, -16(%%ebp)\n\t"
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"subl	-8(%%ebp), %%eax\n\t"
		"sbb	-12(%%ebp), %%edx\n\t"
		"movl	-16(%%ebp), %%ebx\n\t"

		// Store the result (with overhead)
		"movl	%%eax, -20(%%ebp)\n\t"

		// Do everything all over again without the actual step to be measured
		// Start measuring
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"movl	%%eax, -8(%%ebp)\n\t"
		"movl	%%edx, -12(%%ebp)\n\t"

		// Set up the loop
		"movl	$65536, %%ecx\n\t"
		"xorl	%%ebx, %%ebx\n\t"
		"fld	8(%%ebp)\n\t"

	"overheadfpslowloop:\n\t"
		"cmp $0, %%ecx\n\t"
		"je	overheadfpslowout\n\t"
		"decl	%%ecx\n\t"
		"incl	%%ebx\n\t"
		"jmp	overheadfpslowloop\n\t"

		// End measuring
	"overheadfpslowout:\n\t"
		"movl	%%ebx, -16(%%ebp)\n\t"
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"subl	-8(%%ebp), %%eax\n\t"
		"sbb	-12(%%ebp), %%edx\n\t"

		// Subtract the overhead
		"movl	-20(%%ebp), %%ebx\n\t"
		"subl	%%eax, %%ebx\n\t"
		"movl	%%ebx, %%eax\n\t"

		// Store the results
		"movl	%%eax, %0\n\t"
		"movl	-16(%%ebp), %1\n\t"

		// End of routine, pop all that we had pushed
		"popl	%%edi\n\t"
		"popl	%%esi\n\t"
		"popl	%%ebx\n\t"

		"addl	$20, %%esp\n\t"
		"popl	%%ebp\n\t"

		// Remove the initial user parameters
		"addl	$8, %%esp\n\t"
	: "=a" (diff), "=c" (count)
	: "m" (op1), "m" (op2)
	: "edx", "esi"
	);

	return diff / ((double) count*2);
}

double getMisPredictedBranchLatency()
{
	int diff;
	__asm__ volatile(
		// Set up the frame
		"pushl	%%ebp\n\t"
		"movl	%%esp, %%ebp\n\t"
		"subl	$16, %%esp\n\t"

		// Push all regs that we will use in this routine, which is basically all GPRs plus a few others
		"pushl	%%ebx\n\t"
		"pushl	%%esi\n\t"
		"pushl	%%edi\n\t"

		// Start measuring
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"movl	%%eax, -8(%%ebp)\n\t"
		"movl	%%edx, -12(%%ebp)\n\t"

			"jmp	labelM01\n\t"
	"labelM01:	jmp	labelM02\n\t"
	"labelM02:	jmp	labelM03\n\t"
	"labelM03:	jmp	labelM04\n\t"
	"labelM04:	jmp	labelM05\n\t"
	"labelM05:	jmp	labelM06\n\t"
	"labelM06:	jmp	labelM07\n\t"
	"labelM07:	jmp	labelM08\n\t"
	"labelM08:	jmp	labelM09\n\t"
	"labelM09:	jmp	labelM10\n\t"
	"labelM10:	jmp	labelM11\n\t"
	"labelM11:	jmp	labelM12\n\t"
	"labelM12:	jmp	labelM13\n\t"
	"labelM13:	jmp	labelM14\n\t"
	"labelM14:	jmp	labelM15\n\t"
	"labelM15:	jmp	labelM16\n\t"
	"labelM16:	jmp	labelM17\n\t"
	"labelM17:	jmp	labelM18\n\t"
	"labelM18:	jmp	labelM19\n\t"
	"labelM19:	jmp	labelM20\n\t"
	"labelM20:	jmp	labelM21\n\t"
	"labelM21:	jmp	labelM22\n\t"
	"labelM22:	jmp	labelM23\n\t"
	"labelM23:	jmp	labelM24\n\t"
	"labelM24:	jmp	labelM25\n\t"
	"labelM25:	jmp	labelM26\n\t"
	"labelM26:	jmp	labelM27\n\t"
	"labelM27:	jmp	labelM28\n\t"
	"labelM28:	jmp	labelM29\n\t"
	"labelM29:	jmp	labelM30\n\t"
	"labelM30:	jmp	labelM31\n\t"
	"labelM31:	jmp	labelM32\n\t"
	"labelM32:	jmp	labelM33\n\t"
	"labelM33:	jmp	labelM34\n\t"
	"labelM34:	jmp	labelM35\n\t"
	"labelM35:	jmp	labelM36\n\t"
	"labelM36:	jmp	labelM37\n\t"
	"labelM37:	jmp	labelM38\n\t"
	"labelM38:	jmp	labelM39\n\t"
	"labelM39:	jmp	labelM40\n\t"
	"labelM40:	jmp	labelM41\n\t"
	"labelM41:	jmp	labelM42\n\t"
	"labelM42:	jmp	labelM43\n\t"
	"labelM43:	jmp	labelM44\n\t"
	"labelM44:	jmp	labelM45\n\t"
	"labelM45:	jmp	labelM46\n\t"
	"labelM46:	jmp	labelM47\n\t"
	"labelM47:	jmp	labelM48\n\t"
	"labelM48:	jmp	labelM49\n\t"
	"labelM49:	jmp	labelM50\n\t"
	"labelM50:	jmp	labelM51\n\t"
	"labelM51:	jmp	labelM52\n\t"
	"labelM52:	jmp	labelM53\n\t"
	"labelM53:	jmp	labelM54\n\t"
	"labelM54:	jmp	labelM55\n\t"
	"labelM55:	jmp	labelM56\n\t"
	"labelM56:	jmp	labelM57\n\t"
	"labelM57:	jmp	labelM58\n\t"
	"labelM58:	jmp	labelM59\n\t"
	"labelM59:	jmp	labelM60\n\t"
	"labelM60:	jmp	labelM61\n\t"
	"labelM61:	jmp	labelM62\n\t"
	"labelM62:	jmp	labelM63\n\t"
	"labelM63:	jmp	labelM64\n\t"
	"labelM64:\n\t"

		// End measuring
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"subl	-8(%%ebp), %%eax\n\t"
		"sbb	-12(%%ebp), %%edx\n\t"

		// Store the results
		"movl	%%eax, %0\n\t"

		// End of routine, pop all that we had pushed
		"popl	%%edi\n\t"
		"popl	%%esi\n\t"
		"popl	%%ebx\n\t"

		"addl	$16, %%esp\n\t"
		"popl	%%ebp\n\t"
	: "=a" (diff)
	:
	: "ecx", "edx", "esi"
	);

	return diff/64.0;
}

double getPredictedBranchLatency()
{
	int diff;
	__asm__ volatile(
		// Set up the frame
		"pushl	%%ebp\n\t"
		"movl	%%esp, %%ebp\n\t"
		"subl	$20, %%esp\n\t"

		// Push all regs that we will use in this routine, which is basically all GPRs plus a few others
		"pushl	%%ebx\n\t"
		"pushl	%%esi\n\t"
		"pushl	%%edi\n\t"

		// Start measuring
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"movl	%%eax, -8(%%ebp)\n\t"
		"movl	%%edx, -12(%%ebp)\n\t"

		// Set up the loop
		"movl	$65536, %%ecx\n\t"
		"xorl	%%eax, %%eax\n\t"

	"pbranch:\n\t"
		"cmpl	$0, %%ecx\n\t"
		"je	pbranchout\n\t"
		"cmpl	$0, %%eax\n\t"
		"je	pbranchnext\n\t"

	"pbranchnext:\n\t"
		"decl	%%ecx\n\t"
		"jmp	pbranch\n\t"

		// End measuring
	"pbranchout:\n\t"
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"subl	-8(%%ebp), %%eax\n\t"
		"sbb	-12(%%ebp), %%edx\n\t"

		// Store the result (with overhead)
		"movl	%%eax, -20(%%ebp)\n\t"

		// Do everything all over again without the actual step to be measured
		// Start measuring
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"movl	%%eax, -8(%%ebp)\n\t"
		"movl	%%edx, -12(%%ebp)\n\t"

		// Set up the loop
		"movl	$65536, %%ecx\n\t"
		"xorl	%%eax, %%eax\n\t"

	"overheadpbranch:\n\t"
		"cmpl	$0, %%ecx\n\t"
		"je	overheadpbranchout\n\t"
		"decl	%%ecx\n\t"
		"jmp	overheadpbranch\n\t"

		// End measuring
	"overheadpbranchout:\n\t"
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"subl	-8(%%ebp), %%eax\n\t"
		"sbb	-12(%%ebp), %%edx\n\t"

		// Subtract the overhead
		"movl	-20(%%ebp), %%ebx\n\t"
		"subl	%%eax, %%ebx\n\t"
		"movl	%%ebx, %%eax\n\t"

		// Store the results
		"movl	%%eax, %0\n\t"

		// End of routine, pop all that we had pushed
		"popl	%%edi\n\t"
		"popl	%%esi\n\t"
		"popl	%%ebx\n\t"

		"addl	$20, %%esp\n\t"
		"popl	%%ebp\n\t"
	: "=a" (diff)
	:
	: "ecx", "edx", "esi"
	);

	return diff / 65536.0;
}

double getTLBLatency(long size)
{
	int ret, diff, count;
	int stride = getpagesize();
	int warmup = 0;

	unsigned char *ptr;
	if ((ret = posix_memalign((void**) &ptr, 8, size) != 0) || ptr == NULL)
	{
		if (errno == ENOMEM)
			printf ("No memory\n");
		else if (errno == EINVAL)
			printf ("Not aligned\n");
		else	printf ("Unknown error in allocating %ld size with %ld alignment\n", size, size);

		return 0;
	}

	int repeatCount = (warmup == 0 ? 1 : 2);

	__asm__ volatile(
		// Simulate a function call during entry
		"pushl	%2\n\t"		// 16(%%ebp)
		"pushl	%3\n\t"		// 12(%%ebp)
		"pushl	%4\n\t"
		"pushl	%5\n\t"

		// Set up the frame
		"pushl	%%ebp\n\t"
		"movl	%%esp, %%ebp\n\t"
		"subl	$20, %%esp\n\t"

		// Push all regs that we will use in this routine, which is basically all GPRs plus a few others
		"pushl	%%ebx\n\t"
		"pushl	%%esi\n\t"
		"pushl	%%edi\n\t"

		// Setting up repeat counter
		"movl	4(%%ebp), %%ebx\n\t"
		"movl	%%ebx, -4(%%ebp)\n\t"

		"xorl	%%ebx, %%ebx\n\t"
		"movl	16(%%ebp), %%edx\n\t"
		"movl	12(%%ebp), %%ecx\n\t"
		"movl	8(%%ebp), %%esi\n\t"
		"movl	%%edx, %%edi\n\t"
		"addl	%%esi, %%edi\n\t"

	"tlbsetup:\n\t"
		"cmpl	$0, %%ecx\n\t"
		"je	tlbsetupout\n\t"
		"movl	%%edi, (%%edx, %%ebx)\n\t"
		"subl	%%esi, %%ecx\n\t"
		"addl	%%esi, %%ebx\n\t"
		"addl	%%esi, %%edi\n\t"
		"jmp	tlbsetup\n\t"

	"tlbsetupout:\n\t"
		// "Cyclic reference, last location contains address of first location
		"movl	%%edx, (%%edx, %%ebx)\n\t"

		// Change the count of times we loop over this cyclic array
		"movl	12(%%ebp), %%ecx\n\t"
		"addl	%%esi, %%ecx\n\t"

	"tlbshift:\n\t"
		"cmpl	$1, %%esi\n\t"
		"je	tlbshiftout\n\t"
		"shrl	$1, %%esi\n\t"
		"shrl	$1, %%ecx\n\t"
		"jmp	tlbshift\n\t"

	"tlbshiftout:\n\t"
		// Loop a million times
		"movl	%%ecx, 12(%%ebp)\n\t"

	"tlbrepeat:\n\t"
		// Start measuring
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"movl	%%eax, -8(%%ebp)\n\t"
		"movl	%%edx, -12(%%ebp)\n\t"

		// Do some operation
		"xorl	%%ebx, %%ebx\n\t"
		"movl	16(%%ebp), %%edx\n\t"
		"movl	12(%%ebp), %%ecx\n\t"
		"movl	8(%%ebp), %%esi\n\t"

	"tlbwrite:\n\t"
		"cmpl	$0, %%ecx\n\t"
		"je	tlbout\n\t"
		"movl	(%%edx), %%edi\n\t"
		"movl	%%edi, %%edx\n\t"
		"decl	%%ecx\n\t"
		"addl	%%esi, %%ebx\n\t"
		"jmp	tlbwrite\n\t"

	"tlbout:\n\t"
		// End measuring
		"movl	%%ebx, -16(%%ebp)\n\t"
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"subl	-8(%%ebp), %%eax\n\t"
		"sbb	-12(%%ebp), %%edx\n\t"

		// Check if we have to repeat
		"decl	-4(%%ebp)\n\t"
		"jnz	tlbrepeat\n\t"

		// Store the result (with overhead)
		"movl	%%eax, -20(%%ebp)\n\t"

		// TODO: We now have to subtract the cycles spent in TLB hits, so access the last 32 locations,
		// extrapolate to original count and subtract

		// Do everything all over again without the actual step to be measured
		// Start measuring
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"movl	%%eax, -8(%%ebp)\n\t"
		"movl	%%edx, -12(%%ebp)\n\t"

		// Do some operation
		"xorl	%%ebx, %%ebx\n\t"
		"movl	16(%%ebp), %%edx\n\t"
		"movl	12(%%ebp), %%ecx\n\t"
		"movl	8(%%ebp), %%esi\n\t"

	"tlboverheadwrite:\n\t"
		"cmpl	$0, %%ecx\n\t"
		"je	tlboverheadout\n\t"
		"movl	%%edi, %%edx\n\t"
		"decl	%%ecx\n\t"
		"addl	%%esi, %%ebx\n\t"
		"jmp	tlboverheadwrite\n\t"

	"tlboverheadout:\n\t"
		// End measuring
		"movl	%%ebx, -16(%%ebp)\n\t"
		"xorl	%%eax, %%eax\n\t"
		"cpuid\n\t"
		"rdtsc\n\t"
		"subl	-8(%%ebp), %%eax\n\t"
		"sbb	-12(%%ebp), %%edx\n\t"

		// Subtract the overhead
		"movl	-20(%%ebp), %%ebx\n\t"
		"subl	%%eax, %%ebx\n\t"
		"movl	%%ebx, %%eax\n\t"

		// Store the results
		"movl	%%eax, %0\n\t"
		"movl	-16(%%ebp), %1\n\t"

		// End of routine, pop all that we had pushed
		"popl	%%edi\n\t"
		"popl	%%esi\n\t"
		"popl	%%ebx\n\t"

		"addl	$20, %%esp\n\t"
		"popl	%%ebp\n\t"

		// Remove the initial user parameters
		"addl	$16, %%esp\n\t"
	: "=a" (diff), "=c" (count)
	: "m" (ptr), "m" (size), "m" (stride), "m" (repeatCount)
	: "edx", "esi"
	);

	free(ptr);
	//printf ("Count = %d, diff = %d\n", count/stride, diff);

	return diff / ((float) (count / ((float) stride)));
}

#endif /* GENERIC_H_ */
