
#ifndef X86_64_H
#define X86_64_H

#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#define overhead_calc(count, label_prefix)				\
		start_timer()						\
									\
		/* Set up the loop */					\
		"movq	$"#count", %%rcx\n\t"				\
		"xorq	%%rbx, %%rbx\n\t"				\
		"fld	16(%%rbp)\n\t"					\
									\
		core("", label_prefix, overhead, "$1")			\
									\
		/* End measuring */					\
	"overhead"#label_prefix"out:\n\t"				\
		"movq	%%rbx, -32(%%rbp)\n\t"				\
		stop_timer()						\
									\
		/* Subtract the overhead */				\
		"movq	-40(%%rbp), %%rbx\n\t"				\
		"subq	%%rax, %%rbx\n\t"				\
		"movq	%%rbx, %%rax\n\t"				\


#define core(instruction_block, label_prefix, overhead_prefix, incr)	\
	#overhead_prefix#label_prefix"loop:\n\t"			\
		"cmp	$0, %%rcx\n\t"					\
		"je	"#overhead_prefix#label_prefix"out\n\t"		\
		instruction_block					\
		"decq	%%rcx\n\t"					\
		"addq	"incr", %%rbx\n\t"				\
		"jmp	"#overhead_prefix#label_prefix"loop\n\t"


#define measure(init_block, instruction_block, count, label_prefix)	\
		start_timer()						\
									\
		/* Set up the loop */					\
		"movq	$"#count", %%rcx\n\t"				\
		"xorq	%%rbx, %%rbx\n\t"				\
		init_block						\
									\
		core(instruction_block, label_prefix, , "$1")		\
									\
		/* End measuring */					\
	#label_prefix"out:\n\t"						\
		"movq	%%rbx, -32(%%rbp)\n\t"				\
		stop_timer()						\
									\
		/* Store the result (with overhead) */			\
		"movq	%%rax, -40(%%rbp)\n\t"


#define	setup()	/* Create space for stack var and push regs on stack */	\
		"pushq	%%rbp\n\t"					\
		"movq	%%rsp, %%rbp\n\t"				\
		"subq	$40, %%rsp\n\t"					\
									\
		/* Push all regs that we will use in this routine, */	\
		/* which is basically all GPRs plus a few others */	\
		"pushq	%%rbx\n\t"					\
		"pushq	%%rsi\n\t"					\
		"pushq	%%rdi\n\t"


#define cleanup()							\
		 /* Pop all the regs we used */				\
		"popq	%%rdi\n\t"					\
		"popq	%%rsi\n\t"					\
		"popq	%%rbx\n\t"					\
									\
		/* Store the results */					\
		"movq	%%rax, %0\n\t"					\
		"movq	-32(%%rbp), %1\n\t"				\
									\
		/* Restore frame */					\
		"addq	$40, %%rsp\n\t"					\
		"popq	%%rbp\n\t"


#define start_timer()							\
		"xorq	%%rax, %%rax\n\t"				\
		"cpuid\n\t"						\
		"rdtsc\n\t"						\
		"movq	%%rax, -16(%%rbp)\n\t"				\
		"movq	%%rdx, -24(%%rbp)\n\t"				\


#define stop_timer()							\
		"xorq	%%rax, %%rax\n\t"				\
		"cpuid\n\t"						\
		"rdtsc\n\t"						\
		"subq	-16(%%rbp), %%rax\n\t"				\
		"sbb	-24(%%rbp), %%rdx\n\t"


#define measure_mem_with_overhead_calc(label_prefix)			\
		setup()							\
									\
		/* Set up repeat counter */				\
		"movq	32(%%rbp), %%rdx\n\t"				\
		"movq	24(%%rbp), %%rcx\n\t"				\
		"movq	16(%%rbp), %%rsi\n\t"				\
		"movq	8(%%rbp), %%rbx\n\t"				\
		"movq	%%rbx, -8(%%rbp)\n\t"				\
									\
	#label_prefix"repeat:\n\t"					\
		start_timer()						\
									\
		/* Do some operation */					\
		"xorq	%%rbx, %%rbx\n\t"				\
		"movq	32(%%rbp), %%rdx\n\t"				\
		"movq	24(%%rbp), %%rcx\n\t"				\
		"movq	16(%%rbp), %%rsi\n\t"				\
									\
		core							\
		(							\
			"movq	(%%rdx), %%rdi\n\t"			\
			"movq	%%rdi, %%rdx\n\t",			\
			label_prefix,					\
			,						\
			"%%rsi"						\
		)							\
									\
	#label_prefix"out:\n\t"						\
		/* End measuring */					\
		"movq	%%rbx, -32(%%rbp)\n\t"				\
									\
		stop_timer()						\
									\
		/* Check if we have to repeat */			\
		"decq	-8(%%rbp)\n\t"					\
		"jnz	"#label_prefix"repeat\n\t"			\
									\
		/* Store the result (with overhead) */			\
		"movq	%%rax, -40(%%rbp)\n\t"				\
									\
		/* Do everything all over again */			\
		/* without the actual step to be measured */		\
									\
		start_timer()						\
									\
		/* Do some operation */					\
		"xorq	%%rbx, %%rbx\n\t"				\
		"movq	32(%%rbp), %%rdx\n\t"				\
		"movq	24(%%rbp), %%rcx\n\t"				\
		"movq	16(%%rbp), %%rsi\n\t"				\
									\
		core							\
		(							\
			"movq	%%rdi, %%rdx\n\t",			\
			label_prefix,					\
			overhead,					\
			"%%rsi"						\
		)							\
									\
	"overhead"#label_prefix"out:\n\t"				\
		/* End measuring */					\
		"movq	%%rbx, -32(%%rbp)\n\t"				\
									\
		stop_timer()						\
									\
		/* Subtract the overhead */				\
		"movq	-40(%%rbp), %%rbx\n\t"				\
		"subq	%%rax, %%rbx\n\t"				\
		"movq	%%rbx, %%rax\n\t"				\
									\
		cleanup()


#define measure_block_with_overhead_calc(init_block, instruction_block, count, label_prefix)	/* Set up the frame */	\
		setup()								\
		measure(init_block, instruction_block, count, label_prefix)	\
		overhead_calc(count,label_prefix)				\
		cleanup()


#define measure_block_without_overhead_calc(init_block, instruction_block, count, label_prefix)	/* Set up the frame */	\
		setup()								\
		measure(init_block, instruction_block, count, label_prefix)	\
		cleanup()

double allocateAndTest(long size, int stride, unsigned char warmup)
{
	if (stride > size)
		return 0;

	size_t ret = 0, diff = 0, count = 0, my_stride = stride;
	size_t *ptr;

	if ((ret = posix_memalign((void**) &ptr, 8, size) != 0) || ptr == NULL)
	{
		if (errno == ENOMEM)
			printf ("No memory\n");
		else if (errno == EINVAL)
			printf ("Not aligned\n");
		else	printf ("Unknown error in allocating %ld size with %ld alignment\n", size, size);

		return 0;
	}

	int i;
	for (i=0; i<size/sizeof(size_t)-1; i++)
		ptr[i] = (size_t) &(ptr[i+1]);

	ptr[size/sizeof(size_t)-1] = (size_t) &(ptr[0]);

	long loopCount = 1024*1024;
	size_t repeatCount = (warmup == 0 ? 1 : 2);

repeatIfNeg:
	__asm__ volatile(
		// Simulate a function call during entry
		"pushq	%2\n\t"		// 32(%%ebp) ptr
		"pushq	%3\n\t"		// 24(%%ebp) size
		"pushq	%4\n\t"		// 16(%%ebp) stride
		"pushq	%5\n\t"		// 8(%%ebp)  repeatCount

		measure_mem_with_overhead_calc(cache)

		"addq	$32, %%rsp\n\t"

	: "=rax" (diff), "=rcx" (count)
	: "m" (ptr), "m" (loopCount), "m" (my_stride), "m" (repeatCount)
	: "rdx"
	);

	if (((long) diff) < 0)
		goto repeatIfNeg;

	free(ptr);
	return diff / ((float) (count / ((float) stride)));
}

double allocateAndTestMainMemory(long size, int stride, unsigned char warmup)
{
	if (stride > size)
		return 0;

	int i;
	size_t ret = 0, diff = 0, count = 0, my_stride = stride;
	size_t *ptr;

	if ((ret = posix_memalign((void**) &ptr, 8, size) != 0) || ptr == NULL)
	{
		if (errno == ENOMEM)
			printf ("No memory\n");
		else if (errno == EINVAL)
			printf ("Not aligned\n");
		else	printf ("Unknown error in allocating %ld size with %ld alignment\n", size, size);

		return 0;
	}

	long elements = size/stride-1;  // Assuming completely divisible
	int* swapBuffer = malloc(sizeof(int)*elements);
	size_t* mem = (size_t *) ptr;
	if (swapBuffer == NULL)
	{
		// Try the usual +512 skip
		for (i=0; i<elements-1; i++)
		{
			*mem = (size_t) &ptr [(i+1)*stride/sizeof(size_t)];
			mem = (size_t*) *mem;
		}

		// Loop back
		*mem = (size_t) &ptr[0];
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
			*mem = (size_t) &(ptr[randomNumber*stride/sizeof(size_t)]);
			mem = (size_t*) *mem;
		}

		// Loop back to the first location
		*mem = (size_t) ptr;

	       	free(swapBuffer);
	}

	long loopCount = 1024*1024;
	size_t repeatCount = (warmup == 0 ? 1 : 2);

repeatIfNeg:
	__asm__ volatile(
		// Simulate a function call during entry
		"pushq	%2\n\t"		// 32(%%ebp) ptr
		"pushq	%3\n\t"		// 24(%%ebp) size
		"pushq	%4\n\t"		// 16(%%ebp) stride
		"pushq	%5\n\t"		// 8(%%ebp)  repeatCount

		measure_mem_with_overhead_calc(MM)

		"addq	$32, %%rsp\n\t"

	: "=rax" (diff), "=rcx" (count)
	: "m" (ptr), "m" (loopCount), "m" (my_stride), "m" (repeatCount)
	: "rdx"
	);

	if (((long) diff) < 0)
		goto repeatIfNeg;

	free(ptr);
	return diff / ((float) (count / ((float) stride)));
}

double getFPLatency()
{
	size_t diff, count;
	float op1 = 1424.4525;
	float op2 = 0.5636;

	__asm__ volatile(
		// Simulate a function call during entry
		"pushq	%2\n\t"		// 16(%%ebp) ptr
		"pushq	%3\n\t"		// 8(%%ebp) size

		measure_block_with_overhead_calc(
			"fld	16(%%rbp)\n\t",

			"fadd	16(%%rbp)\n\t"
			"fsub	8(%%rbp)\n\t",
			65536, fp)

		"addq	$16, %%rsp\n\t"

	: "=rax" (diff), "=rcx" (count)
	: "m" (op1), "m" (op2)
	: "rdx"
	);

	return diff / ((float) (count * 2));
}

double getFPSlowLatency()
{
	size_t diff, count;
	float op1 = 1424.4525;
	float op2 = 0.5636;

	__asm__ volatile(
		// Simulate a function call during entry
		"pushq	%2\n\t"		// 16(%%ebp) ptr
		"pushq	%3\n\t"		// 8(%%ebp) size

		measure_block_with_overhead_calc(
			"fld	16(%%rbp)\n\t",

			"fdiv	8(%%rbp)\n\t"
			"fsqrt\n\t",
			65536, fpslow)

		"addq	$16, %%rsp\n\t"

	: "=rax" (diff), "=rcx" (count)
	: "m" (op1), "m" (op2)
	: "rdx"
	);

	return diff / ((float) (count * 2));
}

double getPredictedBranchLatency()
{
	size_t diff, count;
	__asm__ volatile(

		measure_block_with_overhead_calc(
			"",
			"cmpq	$0, %%rax\n\t"
			"je	pbranchnext\n\t"
		"pbranchnext:\n\t",
			65536, pbranch)

	: "=rax" (diff), "=rcx" (count)
	:
	: "rdx"
	);

	return diff / 65536.0;
}

double getMisPredictedBranchLatency()
{
	size_t diff, count;
	__asm__ volatile(

		measure_block_without_overhead_calc(
			"",
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
	"labelM64:\n\t",
		1, mpbranch)

	: "=rax" (diff), "=rcx" (count)
	:
	: "rdx"
	);

	return diff / 64.0;
}

double getTLBLatency(long size)
{
	size_t ret = 0, diff = 0, count = 0;
	size_t stride = getpagesize();
	int warmup = 0;

	size_t *ptr;

	if ((ret = posix_memalign((void**) &ptr, 8, size) != 0) || ptr == NULL)
	{
		if (errno == ENOMEM)
			printf ("No memory\n");
		else if (errno == EINVAL)
			printf ("Not aligned\n");
		else	printf ("Unknown error in allocating %ld size with %ld alignment\n", size, size);

		return 0;
	}

	int i, unit_inc = stride/sizeof(size_t);
	for (i=0; i<size/stride-1; i++)
		ptr[i*unit_inc] = (size_t) &(ptr[(i+1)*unit_inc]);

	ptr[(size/stride-1) * unit_inc] = (size_t) &(ptr[0]);

	size = size / stride;
	size_t repeatCount = (warmup == 0 ? 1 : 2);

	__asm__ volatile(
		// Simulate a function call during entry
		"pushq	%2\n\t"		// 32(%%ebp) ptr
		"pushq	%3\n\t"		// 24(%%ebp) size
		"pushq	%4\n\t"		// 16(%%ebp) stride
		"pushq	%5\n\t"		// 8(%%ebp)  repeatCount

		measure_mem_with_overhead_calc(tlb)

		"addq	$32, %%rsp\n\t"

	: "=rax" (diff), "=rcx" (count)
	: "m" (ptr), "m" (size), "m" (stride), "m" (repeatCount)
	: "rdx"
	);

	free(ptr);
	return diff / ((float) (count / ((float) stride)));
}

#endif /* X86_64_H */
