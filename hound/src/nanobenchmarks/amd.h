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

#ifndef NANOBENCHMARKS_AMD_H_
#define NANOBENCHMARKS_AMD_H_

double getAMDBranchLatency()
{
	uint64_t start, end;
	int repeatCount = 50;
	long long cycles = 0;

	int iterationCount;
	volatile short v = 0;
	int array[5] = { 0, 0, 0, 0, 0 };
	int seedIterationCount = 1024/8;
	switch (v)
	{
		case 0:	// Initialize
			iterationCount = seedIterationCount;
			start = rdtsc();

	loop:
		case 1:	asm volatile ("jmp targetBR21\n\ttargetBR21:\n\t");
		case 2:	asm volatile ("jmp targetBR22\n\ttargetBR22:\n\t");
		case 3:	asm volatile ("jmp targetBR23\n\ttargetBR23:\n\t");
		case 4:	asm volatile ("jmp targetBR24\n\ttargetBR24:\n\t");
		case 5:	asm volatile ("jmp targetBR25\n\ttargetBR25:\n\t");
		case 6:	asm volatile ("jmp targetBR26\n\ttargetBR26:\n\t");
		case 7:	asm volatile ("jmp targetBR27\n\ttargetBR27:\n\t");
		case 8:	asm volatile ("jmp targetBR28\n\ttargetBR28:\n\t");

			if (--iterationCount)
				goto loop;

			end = rdtsc();
			cycles = end-start;

			// Subtract overhead
			iterationCount = seedIterationCount;
			start = rdtsc();

		loop2:
			if (--iterationCount)
				goto loop2;
			end = rdtsc();
			long long overhead = end-start;

			return (cycles-overhead)/(seedIterationCount*8.0);
	}

	return -1;
}

#endif /* NANOBENCHMARKS_AMD_H_ */
