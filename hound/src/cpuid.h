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

#include <string.h>

#define __USE_GNU
#include <unistd.h>
#include <sys/types.h>
#include <sched.h>

typedef struct
{
	int reg[4];
} CPUInfo;

#define BOOL int
#define bool int   

enum { EAX=0, EBX, ECX, EDX };

#ifdef __GNUC__
void mycpuid( int * p, unsigned int param, unsigned int ecx )
{
#ifndef	__x86_64
   __asm__ __volatile__
   (
      "pushl	%%ebx\n\t"
      "cpuid\n\t"
      "movl	%%ebx, %1\n\t"
      "popl	%%ebx"
      : "=a" (p[0]), "=r" (p[1]), "=c" (p[2]), "=d" (p[3])
      : "a" (param), "c" (ecx)
      : "cc"
   );
#else
   __asm__ __volatile__
   (
      "cpuid\n\t"
      : "=a" (p[0]), "=b" (p[1]), "=c" (p[2]), "=d" (p[3])
      : "a" (param), "c" (ecx)
      : "cc"
   );
#endif
}
#else /* not __GNUC__ */
#if 0
void mycpuid( int * p, unsigned int param )
{
#ifndef __x86_64
   __asm__ __volatile__
   (
      "mov    %0, %%edi\n\t"
      "cpuid;\n\t"
      "mov    %%eax, 0(%%edi)\n\t"
      "mov    %%ebx, 4(%%edi)\n\t"
      "mov    %%ecx, 8(%%edi)\n\t"
      "mov    %%edx, 12(%%edi)\n\t"
      :
      :"m" (p),"a" (param)
      :"ebx","ecx","edx","edi"
   );
#else
   __asm__ __volatile__
   (
      "movq    %0, %%rdi\n\t"
      "cpuid;\n\t"
      "mov    %%eax, 0(%%rdi)\n\t"
      "mov    %%ebx, 4(%%rdi)\n\t"
      "mov    %%ecx, 8(%%rdi)\n\t"
      "mov    %%edx, 12(%%rdi)\n\t"
      :
      :"m" (p),"a" (param)
      :"ebx","ecx","edx","rdi"
   );
#endif /* __x86_64 */
}
#endif

#endif /* __GNUC__ */

#define __cpuid mycpuid

int isCPUIDSupported()
{
	#ifdef	_NO_CPUID
		return 0;
	#else
		#ifndef	__x86_64
			int supported = 2;
			asm (
				"pushfl\n\t"
				"popl	%%eax\n\t"
				"movl	%%eax, %%ecx\n\t"
				"xorl	$0x200000, %%eax\n\t"
				"pushl	%%eax\n\t"
				"popfl\n\t"

				"pushfl\n\t"
				"popl	%%eax\n\t"
				"xorl	%%ecx, %%eax\n\t"
				"movl	%%eax, %0\n\t"
				: "=r" (supported)
				:: "eax", "ecx"
			);
		#else
			int supported = 2;
			asm (
				"pushfq\n\t"
				"popq   %%rax\n\t"
				"movq   %%rax, %%rcx\n\t"
				"xorq   $0x200000, %%rax\n\t"
				"pushq  %%rax\n\t"
				"popfq\n\t"

				"pushfq\n\t"
				"popq   %%rax\n\t"
				"xorl   %%ecx, %%eax\n\t"
				"movl   %%eax, %0\n\t"
				: "=r" (supported)
				:: "eax", "ecx"
			);
		#endif

        	return supported != 0;
	#endif
}
