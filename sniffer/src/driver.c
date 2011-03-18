
#include <papi.h>
#include <stdio.h>
#include <string.h>

#define	BOOL			unsigned char
#define	COUNTER_NOT_PRESENT	((BOOL) 0)
#define	COUNTER_PRESENT		((BOOL) 1)
#define	COUNTER_USED		((BOOL) 2)
#define	COUNTER_ADDED		((BOOL) 3)

enum { TOT_INS=0, TOT_CYC, L1_DCA, L2_DCA, L2_TCA, L1_ICA, L2_ICA, L2_DCM, L2_ICM, L2_TCM, TLB_DM, TLB_IM, BR_INS, BR_MSP, FP_INS, FML_INS, FDV_INS, FAD_INS, L1I_CYCLES_STALLED, ARITH_CYCLES_DIV_BUSY, FP_COMP_OPS_EXE_SSE_SINGLE_PRECISION, FP_COMP_OPS_EXE_X87, FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION, FP_COMP_OPS_EXE_SSE_FP, FP_COMP_OPS_EXE_SSE_FP_PACKED, FP_COMP_OPS_EXE_SSE_FP_SCALAR, FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION_SSE_FP_SSE_FP_PACKED_SSE_FP_SCALAR_SSE_SINGLE_PRECISION_X87, ENUM_LENGTH };

#define	USE_COUNTER(X)		(counter_present[X] = COUNTER_USED)
#define	ADD_COUNTER(X)		(counter_present[X] = COUNTER_ADDED)
#define	DELETE_COUNTER(X)	(counter_present[X] = COUNTER_NOT_PRESENT)

#define	IS_COUNTER_AVAILABLE(X)	(counter_present[X] != COUNTER_NOT_PRESENT)
#define	IS_COUNTER_USED(X)	(counter_present[X] == COUNTER_USED)
#define	IS_COUNTER_ADDED(X)	(counter_present[X] == COUNTER_ADDED)

#define	LCPI_DEF_LENGTH	300

char* counter_names [ENUM_LENGTH] = { "PAPI_TOT_INS", "PAPI_TOT_CYC", "PAPI_L1_DCA", "PAPI_L2_DCA", "PAPI_L2_TCA", "PAPI_L1_ICA", "PAPI_L2_ICA", "PAPI_L2_DCM", "PAPI_L2_ICM", "PAPI_L2_TCM", "PAPI_TLB_DM", "PAPI_TLB_IM", "PAPI_BR_INS", "PAPI_BR_MSP", "PAPI_FP_INS", "PAPI_FML_INS", "PAPI_FDV_INS", "PAPI_FAD_INS", "L1I_CYCLES_STALLED", "ARITH:CYCLES_DIV_BUSY", "FP_COMP_OPS_EXE:SSE_SINGLE_PRECISION", "FP_COMP_OPS_EXE:X87", "FP_COMP_OPS_EXE:SSE_DOUBLE_PRECISION", "FP_COMP_OPS_EXE:SSE_FP", "FP_COMP_OPS_EXE:SSE_FP_PACKED", "FP_COMP_OPS_EXE:SSE_FP_SCALAR", "FP_COMP_OPS_EXE:SSE_DOUBLE_PRECISION:SSE_FP:SSE_FP_PACKED:SSE_FP_SCALAR:SSE_SINGLE_PRECISION:X87" };

int counter_freq [ENUM_LENGTH] = { 13000027, 13000049, 5300003, 1300021, 1300031, 5300027, 1300051, 1300073, 1300111, 1300843, 510007, 510031, 2300017, 510047, 5300081, 5300033, 510049, 5300063, 510067, 510079, 3000017, 3000029, 3000047, 3000061, 3000073, 3000077, 510101 };

BOOL counter_present[ENUM_LENGTH]={0};

void counter_err(char* lcpi)
{
	fprintf(stderr, "********************************************************************************\nERROR: Required counters not found for LCPI: %s\n********************************************************************************\n", lcpi);
}

void mark_available_counter(char* counter_name)
{
	// Loop over all required counters
	int j;
	for (j=0; j<ENUM_LENGTH; j++)
		if (strcmp(counter_names[j], counter_name) == 0)
			counter_present [j] = COUNTER_PRESENT;
}

int is_available(PAPI_event_info_t* info)
{
	return info->count;
}

// From PAPI source
int is_derived(PAPI_event_info_t* info )
{
	return strlen(info->derived) != 0 && strcmp(info->derived, "NOT_DERIVED") != 0 &&
		strcmp(info->derived, "DERIVED_CMPD") != 0;
}

int main(int argc, char* argv [])
{
	if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT)
	{
		fprintf (stderr, "Failed initializing PAPI\n");
		return 1;
	}

	int i=0 | PAPI_PRESET_MASK, j, k;
	PAPI_event_info_t info;
	PAPI_enum_event(&i, PAPI_ENUM_FIRST);

	do
	{
		if (PAPI_get_event_info(i, &info) == PAPI_OK && is_available(&info) && !is_derived(&info))
			mark_available_counter(info.symbol);
	} while (PAPI_enum_event(&i, PAPI_PRESET_ENUM_AVAIL) == PAPI_OK);

	i=0 | PAPI_NATIVE_MASK;
	PAPI_enum_event(&i, PAPI_ENUM_FIRST);

	do {
		if (PAPI_get_event_info(i, &info) == PAPI_OK)
		{
			k = i;
			if (PAPI_enum_event(&k, PAPI_NTV_ENUM_UMASKS) == PAPI_OK)
			{
				do
				{
					if (PAPI_get_event_info(k, &info) == PAPI_OK)
						mark_available_counter(info.symbol);
				} while ( PAPI_enum_event( &k, PAPI_NTV_ENUM_UMASKS ) == PAPI_OK );
			}
			else
				mark_available_counter(info.symbol);
		}
	} while (PAPI_enum_event(&i, PAPI_ENUM_EVENTS) == PAPI_OK);

	// Check the basic ones first, so that we don't have to repeat their checks every time we use them
	if (!IS_COUNTER_AVAILABLE(TOT_CYC))
		counter_err("PAPI_TOT_CYC");
	USE_COUNTER(TOT_CYC);

	if (!IS_COUNTER_AVAILABLE(TOT_INS))
		counter_err("PAPI_TOT_INS");
	USE_COUNTER(TOT_INS);

	// ratio.floating_point
	char ratio_floating_point [LCPI_DEF_LENGTH];
	if (IS_COUNTER_AVAILABLE(FML_INS) && IS_COUNTER_AVAILABLE(FDV_INS))
	{
		USE_COUNTER(FML_INS);
		USE_COUNTER(FDV_INS);

		if (IS_COUNTER_AVAILABLE(FAD_INS))
		{
			USE_COUNTER(FAD_INS);
			strcpy(ratio_floating_point, "ratio.floating_point = (PAPI_FML_INS + PAPI_FDV_INS + PAPI_FAD_INS) / PAPI_TOT_INS\n");
		}
		else
			strcpy(ratio_floating_point, "ratio.floating_point = (PAPI_FML_INS + PAPI_FDV_INS) / PAPI_TOT_INS\n");
	}
	else if (IS_COUNTER_AVAILABLE(FP_INS))
	{
		USE_COUNTER(FP_INS);
		strcpy(ratio_floating_point, "ratio.floating_point = PAPI_FP_INS / PAPI_TOT_INS\n");
	}
	else	counter_err("ratio.floating_point");

	//ratio.data_accesses
	char ratio_data_accesses [LCPI_DEF_LENGTH];
	if (IS_COUNTER_AVAILABLE(L1_DCA))
	{
		USE_COUNTER(L1_DCA);
		strcpy(ratio_data_accesses, "ratio.data_accesses = PAPI_L1_DCA / PAPI_TOT_INS\n\n");
	}
	else	counter_err("data_accesses");

	//overall
	char overall [LCPI_DEF_LENGTH];
	strcpy(overall, "overall = PAPI_TOT_CYC / PAPI_TOT_INS\n\n");

	//data_accesses
	char lcpi_L2_DCA [LCPI_DEF_LENGTH];
	char lcpi_L2_DCM [LCPI_DEF_LENGTH];

	if (IS_COUNTER_AVAILABLE(L2_DCA))
	{
		USE_COUNTER(L2_DCA);
		strcpy(lcpi_L2_DCA, "PAPI_L2_DCA");
	}
	else if (IS_COUNTER_AVAILABLE(L2_TCA) && IS_COUNTER_AVAILABLE(L2_ICA))
	{
		USE_COUNTER(L2_TCA);
		USE_COUNTER(L2_ICA);
		strcpy(lcpi_L2_DCA, "(PAPI_L2_TCA - PAPI_L2_ICA)");
	}
	else	counter_err("PAPI_L2_DCA");

	if (IS_COUNTER_AVAILABLE(L2_DCM))
	{
		USE_COUNTER(L2_DCM);
		strcpy(lcpi_L2_DCM, "PAPI_L2_DCM");
	}
	else if (IS_COUNTER_AVAILABLE(L2_TCM) && IS_COUNTER_AVAILABLE(L2_ICM))
	{
		USE_COUNTER(L2_TCM);
		USE_COUNTER(L2_ICM);
		strcpy(lcpi_L2_DCM, "(PAPI_L2_TCM - PAPI_L2_ICM)");
	}
	else	counter_err("PAPI_L2_DCM");

	char data_accesses_overall [LCPI_DEF_LENGTH];
	sprintf(data_accesses_overall, "data_accesses.overall = (PAPI_L1_DCA * L1_dlat + %s * L2_lat + %s * mem_lat) / PAPI_TOT_INS\n", lcpi_L2_DCA, lcpi_L2_DCM);

	char data_accesses_L1d_hits [LCPI_DEF_LENGTH];
	strcpy(data_accesses_L1d_hits, "data_accesses.L1d_hits = (PAPI_L1_DCA * L1_dlat) / PAPI_TOT_INS\n");

	char data_accesses_L2d_hits [LCPI_DEF_LENGTH];
	sprintf(data_accesses_L2d_hits, "data_accesses.L2d_hits = (%s * L2_lat) / PAPI_TOT_INS\n", lcpi_L2_DCA);

	char data_accesses_L2d_misses [LCPI_DEF_LENGTH];
	sprintf(data_accesses_L2d_misses, "data_accesses.L2d_misses = (%s * mem_lat) / PAPI_TOT_INS\n\n", lcpi_L2_DCM);

	//instruction_accesses
	char lcpi_L2_ICA [LCPI_DEF_LENGTH];
	char lcpi_L2_ICM [LCPI_DEF_LENGTH];

	if (!IS_COUNTER_AVAILABLE(L1_ICA))
		counter_err("PAPI_L1_ICA");
	USE_COUNTER(L1_ICA);

	if (IS_COUNTER_AVAILABLE(L2_ICA))
	{
		USE_COUNTER(L2_ICA);
		strcpy(lcpi_L2_ICA, "PAPI_L2_ICA");
	}
	else if (IS_COUNTER_AVAILABLE(L2_TCA) && IS_COUNTER_AVAILABLE(L2_DCA))
	{
		USE_COUNTER(L2_TCA);
		USE_COUNTER(L2_DCA);
		strcpy(lcpi_L2_ICA, "(PAPI_L2_TCA - PAPI_L2_DCA)");
	}
	else	counter_err("PAPI_L2_ICA");

	if (IS_COUNTER_AVAILABLE(L2_ICM))
	{
		USE_COUNTER(L2_ICM);
		strcpy(lcpi_L2_ICM, "PAPI_L2_ICM");
	}
	else if (IS_COUNTER_AVAILABLE(L2_TCM) && IS_COUNTER_AVAILABLE(L2_DCM))
	{
		USE_COUNTER(L2_TCM);
		USE_COUNTER(L2_DCM);
		strcpy(lcpi_L2_ICM, "(PAPI_L2_TCM - PAPI_L2_DCM)");
	}
	else	counter_err("PAPI_L2_ICM");

	char instruction_accesses_overall [LCPI_DEF_LENGTH];
	sprintf(instruction_accesses_overall, "instruction_accesses.overall = (PAPI_L1_ICA * L1_ilat + %s * L2_lat + %s * mem_lat) / PAPI_TOT_INS\n", lcpi_L2_ICA, lcpi_L2_ICM);

	char instruction_accesses_L1i_hits [LCPI_DEF_LENGTH];
	strcpy(instruction_accesses_L1i_hits, "instruction_accesses.L1i_hits = PAPI_L1_ICA * L1_ilat / PAPI_TOT_INS\n");

	char instruction_accesses_L2i_hits [LCPI_DEF_LENGTH];
	sprintf(instruction_accesses_L2i_hits, "instruction_accesses.L2i_hits = %s * L2_lat / PAPI_TOT_INS\n", lcpi_L2_ICA);

	char instruction_accesses_L2i_misses [LCPI_DEF_LENGTH];
	sprintf(instruction_accesses_L2i_misses, "instruction_accesses.L2i_misses = %s * mem_lat / PAPI_TOT_INS\n\n", lcpi_L2_ICM);

	//data_TLB
	char data_TLB_overall [LCPI_DEF_LENGTH];
	if (IS_COUNTER_AVAILABLE(TLB_DM))
	{
		USE_COUNTER(TLB_DM);
		strcpy(data_TLB_overall, "data_TLB.overall = PAPI_TLB_DM * TLB_lat / PAPI_TOT_INS\n");
	}
	else	counter_err("PAPI_TLB_DM");

	//instruction_TLB
	char instruction_TLB_overall [LCPI_DEF_LENGTH];
	if (IS_COUNTER_AVAILABLE(TLB_IM))
	{
		USE_COUNTER(TLB_IM);
		strcpy(instruction_TLB_overall, "instruction_TLB.overall = PAPI_TLB_IM * TLB_lat / PAPI_TOT_INS\n\n");
	}
	else	counter_err("PAPI_TLB_IM");

	//branch_instructions
	char branch_instructions_overall [LCPI_DEF_LENGTH];
	char branch_instructions_correctly_predicted [LCPI_DEF_LENGTH];
	char branch_instructions_mispredicted [LCPI_DEF_LENGTH];

	if (IS_COUNTER_AVAILABLE(BR_INS) && IS_COUNTER_AVAILABLE(BR_MSP))
	{
		USE_COUNTER(BR_INS);
		USE_COUNTER(BR_MSP);
		strcpy(branch_instructions_overall, "branch_instructions.overall = (PAPI_BR_INS * BR_lat + PAPI_BR_MSP * BR_miss_lat) / PAPI_TOT_INS\n");
		strcpy(branch_instructions_correctly_predicted, "branch_instructions.correctly_predicted = PAPI_BR_INS * BR_lat / PAPI_TOT_INS\n");
		strcpy(branch_instructions_mispredicted, "branch_instructions.mispredicted = PAPI_BR_MSP * BR_miss_lat / PAPI_TOT_INS\n\n");
	}
	else
		counter_err("branch_instructions");

	//floating-point_instr
	char lcpi_slow [LCPI_DEF_LENGTH];
	char lcpi_fast [LCPI_DEF_LENGTH];

	if (IS_COUNTER_AVAILABLE(FML_INS))
	{
		USE_COUNTER(FML_INS);
		if (IS_COUNTER_AVAILABLE(FAD_INS))
		{
			USE_COUNTER(FAD_INS);
			strcpy(lcpi_fast, "((PAPI_FML_INS + PAPI_FAD_INS) * FP_lat)");
		}
		else
			strcpy(lcpi_fast, "(PAPI_FML_INS * FP_lat)");
	}
	else if (IS_COUNTER_AVAILABLE(FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION) && IS_COUNTER_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP) && IS_COUNTER_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP_PACKED) && IS_COUNTER_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP_SCALAR) && IS_COUNTER_AVAILABLE(FP_COMP_OPS_EXE_SSE_SINGLE_PRECISION) && IS_COUNTER_AVAILABLE(FP_COMP_OPS_EXE_X87))
	{
		// Do a dummy test to see if the single big counter can be added or do we have to break it down into multiple parts
		int event_set = PAPI_NULL;
		if (PAPI_create_eventset(&event_set) != PAPI_OK)
		{
			fprintf (stderr, "Could not create PAPI event set for determining which counters can be counted simultaneously\n");
			return 1;
		}

		// Add PAPI_TOT_INS as we will be measuring it in every run
		int papi_tot_ins_code;

		PAPI_event_name_to_code("PAPI_TOT_INS", &papi_tot_ins_code);
		if (PAPI_add_event(event_set, papi_tot_ins_code) != PAPI_OK)
		{
			fprintf (stderr, "Could not add PAPI_TOT_INS to the event set, cannot continue...\n");
			return 1;
		}

		int event_code;
		PAPI_event_name_to_code("FP_COMP_OPS_EXE:SSE_DOUBLE_PRECISION:SSE_FP:SSE_FP_PACKED:SSE_FP_SCALAR:SSE_SINGLE_PRECISION:X87", &event_code);
		if (PAPI_add_event(event_set, event_code) == PAPI_OK)
		{
			// Good! It can be added
			USE_COUNTER(FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION_SSE_FP_SSE_FP_PACKED_SSE_FP_SCALAR_SSE_SINGLE_PRECISION_X87);
			DELETE_COUNTER(FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION);
			DELETE_COUNTER(FP_COMP_OPS_EXE_SSE_FP);
			DELETE_COUNTER(FP_COMP_OPS_EXE_SSE_FP_PACKED);
			DELETE_COUNTER(FP_COMP_OPS_EXE_SSE_FP_SCALAR);
			DELETE_COUNTER(FP_COMP_OPS_EXE_SSE_SINGLE_PRECISION);
			DELETE_COUNTER(FP_COMP_OPS_EXE_X87);

			strcpy(lcpi_fast, "(FP_COMP_OPS_EXE:SSE_DOUBLE_PRECISION:SSE_FP:SSE_FP_PACKED:SSE_FP_SCALAR:SSE_SINGLE_PRECISION:X87 * FP_lat)");
		}
		else
		{
			// Asume that we can add separately. If this is false, we will catch it at the time of generating LCPIs

			// Use the available counters
			USE_COUNTER(FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION);
			USE_COUNTER(FP_COMP_OPS_EXE_SSE_FP);
			USE_COUNTER(FP_COMP_OPS_EXE_SSE_FP_PACKED);
			USE_COUNTER(FP_COMP_OPS_EXE_SSE_FP_SCALAR);
			USE_COUNTER(FP_COMP_OPS_EXE_SSE_SINGLE_PRECISION);
			USE_COUNTER(FP_COMP_OPS_EXE_X87);

			strcpy(lcpi_fast, "((FP_COMP_OPS_EXE:SSE_DOUBLE_PRECISION + FP_COMP_OPS_EXE:SSE_FP + FP_COMP_OPS_EXE:SSE_FP_PACKED + FP_COMP_OPS_EXE:SSE_FP_SCALAR + FP_COMP_OPS_EXE:SSE_SINGLE_PRECISION + FP_COMP_OPS_EXE:X87) * FP_lat)");
		}
	}
	else	counter_err("floating-point_instr.fast_FP_inst");

	if (IS_COUNTER_AVAILABLE(FDV_INS))
	{
		USE_COUNTER(FDV_INS);
		strcpy(lcpi_slow, "(PAPI_FDV_INS * FP_slow_lat)");
	}
	else if (IS_COUNTER_AVAILABLE(ARITH_CYCLES_DIV_BUSY))
	{
		USE_COUNTER(ARITH_CYCLES_DIV_BUSY);
		strcpy(lcpi_slow, "ARITH:CYCLES_DIV_BUSY");
	}
	else	counter_err("floating-point_instr.slow_FP_inst");

	char floating_point_instr_overall [LCPI_DEF_LENGTH];
	sprintf(floating_point_instr_overall, "floating-point_instr.overall = (%s + %s) / PAPI_TOT_INS\n", lcpi_fast, lcpi_slow);

	char floating_point_instr_fast_FP_instr [LCPI_DEF_LENGTH];
	sprintf(floating_point_instr_fast_FP_instr, "floating-point_instr.fast_FP_instr = %s / PAPI_TOT_INS\n", lcpi_fast);

	char floating_point_instr_slow_FP_instr [LCPI_DEF_LENGTH];
	sprintf(floating_point_instr_slow_FP_instr, "floating-point_instr.slow_FP_instr = %s / PAPI_TOT_INS\n", lcpi_slow);

	int event_code;
	int exp_count = 0;
	int papi_tot_ins_code;

	short addCount, remaining;
	PAPI_event_name_to_code("PAPI_TOT_INS", &papi_tot_ins_code);

	#define MAX_REL_PRIMES	7
	int rel_primes [MAX_REL_PRIMES] = { 510007, 510009, 530003, 510013, 530006, 510017, 510019 };

	FILE* fp = fopen ("experiment.header.tmp", "w");
	if (fp == NULL)
	{
		fprintf (stderr, "Could not open file experiment.header for writing, terminaing...\n");
		return 1;
	}

	do
	{
		int event_set = PAPI_NULL;
		if (PAPI_create_eventset(&event_set) != PAPI_OK)
		{
			fprintf (stderr, "Could not create PAPI event set for determining which counters can be counted simultaneously\n");
			return 1;
		}

		// Add PAPI_TOT_INS as we will be measuring it in every run
		if (PAPI_add_event(event_set, papi_tot_ins_code) != PAPI_OK)
		{
			fprintf (stderr, "Could not add PAPI_TOT_INS to the event set, cannot continue...\n");
			return 1;
		}

		addCount = 0;
		remaining = 0;
		short recently_added [MAX_REL_PRIMES] = { -1, -1, -1, -1, -1, -1, -1 };
		for (j=0; j<ENUM_LENGTH; j++)
		{
			// Don't need to specially add PAPI_TOT_INS, we are measuring that anyway
			if (j == PAPI_TOT_INS)
				continue;

			if (counter_present[j] == COUNTER_USED && j != TOT_INS)
			{
				PAPI_event_name_to_code(counter_names[j], &event_code);
				if (PAPI_add_event(event_set, event_code) == PAPI_OK)
				{
					ADD_COUNTER(j);
					recently_added[addCount] = j;

					if (remaining == 0)	// New line
						fprintf (fp, "experiment[%d]=\\\"", exp_count);

					fprintf (fp, "--event %s:%d ", counter_names[j], counter_freq[j]);
					addCount++;
				}

				remaining++;

				// If we've used all the relatively prime numbers, break into the next experiment
				if (remaining == MAX_REL_PRIMES-1)
					break;
			}
		}

		if (addCount == 0 && remaining > 0)	// Some events remain but could not be added to the event set
		{
			fprintf (stderr, "ERROR: The following events are available but could not be added to the event set:\n");
			for (j=0; j<ENUM_LENGTH; j++)
				if (counter_present[j] == COUNTER_USED)
					fprintf (stderr, "%s ", counter_names[j]);

			fprintf (stderr, "\n");
			break;
		}
		else
			exp_count++;

		if (addCount > 0 && remaining > 0)	fprintf (fp, "--event PAPI_TOT_INS:%d\\\"\\n", counter_freq[TOT_INS]);
	} while (remaining > 0);

	fclose(fp);

	fp = fopen ("lcpi.properties", "w");
	if (fp == NULL)
	{
		fprintf (stderr, "Could not open file lcpi.properties for writing, terminating...\n");
		return 1;
	}

	fprintf (fp, "version = 1.0\n\n# LCPI config generated using sniffer\n");
	fprintf (fp, "%s", ratio_floating_point);
	fprintf (fp, "%s", ratio_data_accesses);

	fprintf (fp, "%s", overall);

	fprintf (fp, "%s", data_accesses_overall);
	fprintf (fp, "%s", data_accesses_L1d_hits);
	fprintf (fp, "%s", data_accesses_L2d_hits);
	fprintf (fp, "%s", data_accesses_L2d_misses);

	fprintf (fp, "%s", instruction_accesses_overall);
	fprintf (fp, "%s", instruction_accesses_L1i_hits);
	fprintf (fp, "%s", instruction_accesses_L2i_hits);
	fprintf (fp, "%s", instruction_accesses_L2i_misses);

	fprintf (fp, "%s", data_TLB_overall);
	fprintf (fp, "%s", instruction_TLB_overall);

	fprintf (fp, "%s", branch_instructions_overall);
	fprintf (fp, "%s", branch_instructions_correctly_predicted);
	fprintf (fp, "%s", branch_instructions_mispredicted);

	fprintf (fp, "%s", floating_point_instr_overall);
	fprintf (fp, "%s", floating_point_instr_fast_FP_instr);
	fprintf (fp, "%s", floating_point_instr_slow_FP_instr);

	fclose (fp);

	fprintf (stderr, "Generated LCPI and experiment files\n");
	return 0;
}
