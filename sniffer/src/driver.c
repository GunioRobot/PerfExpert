
#include <papi.h>
#include <stdio.h>
#include <string.h>

#define	BOOL			unsigned char
#define	COUNTER_NOT_PRESENT	((BOOL) 0)
#define	COUNTER_PRESENT		((BOOL) 1)
#define	COUNTER_USED		((BOOL) 2)
#define	COUNTER_ADDED		((BOOL) 3)

enum { TOT_INS=0, TOT_CYC, L1_DCA, L2_DCA, L2_TCA, L1_ICA, L2_ICA, L2_DCM, L2_ICM, L2_TCM, TLB_DM, TLB_IM, BR_INS, BR_MSP, FP_INS, FML_INS, FDV_INS, FAD_INS, L1I_CYCLES_STALLED, FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION, FP_COMP_OPS_EXE_SSE_FP, FP_COMP_OPS_EXE_SSE_FP_PACKED, FP_COMP_OPS_EXE_SSE_FP_SCALAR, FP_COMP_OPS_EXE_SSE_SINGLE_PRECISION, FP_COMP_OPS_EXE_X87, ARITH_CYCLES_DIV_BUSY, FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION_SSE_FP_SSE_FP_PACKED_SSE_FP_SCALAR_SSE_SINGLE_PRECISION_X87, ENUM_LENGTH };

#define	USE_COUNTER(X)		(counter_present[X] = COUNTER_USED)
#define	DELETE_COUNTER(X)	(counter_present[X] = COUNTER_NOT_PRESENT)
#define	COUNTER_AVAILABLE(X)	(counter_present[X] >= COUNTER_PRESENT)

char* counter_names [ENUM_LENGTH] = { "PAPI_TOT_INS", "PAPI_TOT_CYC", "PAPI_L1_DCA", "PAPI_L2_DCA", "PAPI_L2_TCA", "PAPI_L1_ICA", "PAPI_L2_ICA", "PAPI_L2_DCM", "PAPI_L2_ICM", "PAPI_L2_TCM", "PAPI_TLB_DM", "PAPI_TLB_IM", "PAPI_BR_INS", "PAPI_BR_MSP", "PAPI_FP_INS", "PAPI_FML_INS", "PAPI_FDV_INS", "PAPI_FAD_INS", "L1I_CYCLES_STALLED", "FP_COMP_OPS_EXE:SSE_DOUBLE_PRECISION", "FP_COMP_OPS_EXE:SSE_FP", "FP_COMP_OPS_EXE:SSE_FP_PACKED", "FP_COMP_OPS_EXE:SSE_FP_SCALAR", "FP_COMP_OPS_EXE:SSE_SINGLE_PRECISION", "FP_COMP_OPS_EXE:X87", "ARITH:CYCLES_DIV_BUSY", "FP_COMP_OPS_EXE:SSE_DOUBLE_PRECISION:SSE_FP:SSE_FP_PACKED:SSE_FP_SCALAR:SSE_SINGLE_PRECISION:X87" };

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
	if (!COUNTER_AVAILABLE(TOT_CYC))
		counter_err("PAPI_TOT_CYC");
	USE_COUNTER(TOT_CYC);

	if (!COUNTER_AVAILABLE(TOT_INS))
		counter_err("PAPI_TOT_INS");
	USE_COUNTER(TOT_INS);

	FILE* fp = fopen ("lcpi.properties", "w");
	if (fp == NULL)
	{
		fprintf (stderr, "Could not open file lcpi.properties for writing, terminating...\n");
		return 1;
	}

	fprintf(fp, "version = 1.0\n\n# LCPI config generated using sniffer\n");

	// ratio.floating_point
	if (COUNTER_AVAILABLE(FML_INS) && COUNTER_AVAILABLE(FDV_INS))
	{
		USE_COUNTER(FML_INS);
		USE_COUNTER(FDV_INS);

		if (COUNTER_AVAILABLE(FAD_INS))
		{
			USE_COUNTER(FAD_INS);
			fprintf (fp, "ratio.floating_point = (PAPI_FML_INS + PAPI_FDV_INS + PAPI_FAD_INS) / PAPI_TOT_INS\n");
		}
		else
			fprintf (fp, "ratio.floating_point = (PAPI_FML_INS + PAPI_FDV_INS) / PAPI_TOT_INS\n");
	}
	else if (COUNTER_AVAILABLE(FP_INS))
	{
		USE_COUNTER(FP_INS);
		fprintf (fp, "ratio.floating_point = PAPI_FP_INS / PAPI_TOT_INS\n");
	}
	else	counter_err("ratio.floating_point");

	//ratio.data_accesses
	if (COUNTER_AVAILABLE(L1_DCA))
	{
		USE_COUNTER(L1_DCA);
		fprintf (fp, "ratio.data_accesses = PAPI_L1_DCA / PAPI_TOT_INS\n\n");
	}
	else	counter_err("data_accesses");

	//overall
	fprintf (fp, "overall = PAPI_TOT_CYC / PAPI_TOT_INS\n\n");

	//data_accesses
	char* lcpi_L2_DCA = NULL;
	char* lcpi_L2_DCM = NULL;

	if (COUNTER_AVAILABLE(L2_DCA))
	{
		USE_COUNTER(L2_DCA);
		lcpi_L2_DCA = "PAPI_L2_DCA";
	}
	else if (COUNTER_AVAILABLE(L2_TCA) && COUNTER_AVAILABLE(L2_ICA))
	{
		USE_COUNTER(L2_TCA);
		USE_COUNTER(L2_ICA);
		lcpi_L2_DCA = "(PAPI_L2_TCA - PAPI_L2_ICA)";
	}
	else	counter_err("PAPI_L2_DCA");

	if (COUNTER_AVAILABLE(L2_DCM))
	{
		USE_COUNTER(L2_DCM);
		lcpi_L2_DCM = "PAPI_L2_DCM";
	}
	else if (COUNTER_AVAILABLE(L2_TCM) && COUNTER_AVAILABLE(L2_ICM))
	{
		USE_COUNTER(L2_TCM);
		USE_COUNTER(L2_ICM);
		lcpi_L2_DCM = "(PAPI_L2_TCM - PAPI_L2_ICM)";
	}
	else	counter_err("PAPI_L2_DCM");

	fprintf (fp, "data_accesses.overall = (PAPI_L1_DCA * L1_dlat + %s * L2_lat + %s * mem_lat) / PAPI_TOT_INS\n", lcpi_L2_DCA, lcpi_L2_DCM);
	fprintf (fp, "data_accesses.L1d_hits = (PAPI_L1_DCA * L1_dlat) / PAPI_TOT_INS\n");
	fprintf (fp, "data_accesses.L2d_hits = (%s * L2_lat) / PAPI_TOT_INS\n", lcpi_L2_DCA);
	fprintf (fp, "data_accesses.L2d_misses = (%s * mem_lat) / PAPI_TOT_INS\n\n", lcpi_L2_DCM);

	//instruction_accesses
	char* lcpi_L2_ICA = NULL;
	char* lcpi_L2_ICM = NULL;

	if (!COUNTER_AVAILABLE(L1_ICA))
		counter_err("PAPI_L1_ICA");
	USE_COUNTER(L1_ICA);

	if (COUNTER_AVAILABLE(L2_ICA))
	{
		USE_COUNTER(L2_ICA);
		lcpi_L2_ICA = "PAPI_L2_ICA";
	}
	else if (COUNTER_AVAILABLE(L2_TCA) && COUNTER_AVAILABLE(L2_DCA))
	{
		USE_COUNTER(L2_TCA);
		USE_COUNTER(L2_DCA);
		lcpi_L2_ICA = "(PAPI_L2_TCA - PAPI_L2_DCA)";
	}
	else	counter_err("PAPI_L2_ICA");

	if (COUNTER_AVAILABLE(L2_ICM))
	{
		USE_COUNTER(L2_ICM);
		lcpi_L2_ICM = "PAPI_L2_ICM";
	}
	else if (COUNTER_AVAILABLE(L2_TCM) && COUNTER_AVAILABLE(L2_DCM))
	{
		USE_COUNTER(L2_TCM);
		USE_COUNTER(L2_DCM);
		lcpi_L2_ICM = "(PAPI_L2_TCM - PAPI_L2_DCM)";
	}
	else	counter_err("PAPI_L2_ICM");

	fprintf (fp, "instruction_accesses.overall = (PAPI_L1_ICA * L1_ilat + %s * L2_lat + %s * mem_lat) / PAPI_TOT_INS\n", lcpi_L2_ICA, lcpi_L2_ICM);
	fprintf (fp, "instruction_accesses.L1i_hits = PAPI_L1_ICA * L1_ilat / PAPI_TOT_INS\n", lcpi_L2_ICA, lcpi_L2_ICM);
	fprintf (fp, "instruction_accesses.L2i_hits = %s * L2_lat / PAPI_TOT_INS\n", lcpi_L2_ICA);
	fprintf (fp, "instruction_accesses.L2i_misses = %s * mem_lat / PAPI_TOT_INS\n\n", lcpi_L2_ICM);

	//data_TLB
	if (COUNTER_AVAILABLE(TLB_DM))
	{
		USE_COUNTER(TLB_DM);
		fprintf (fp, "data_TLB.overall = PAPI_TLB_DM * TLB_lat / PAPI_TOT_INS\n");
	}
	else	counter_err("PAPI_TLB_DM");

	//instruction_TLB
	if (COUNTER_AVAILABLE(TLB_IM))
	{
		USE_COUNTER(TLB_IM);
		fprintf (fp, "instruction_TLB.overall = PAPI_TLB_IM * TLB_lat / PAPI_TOT_INS\n\n");
	}
	else	counter_err("PAPI_TLB_IM");

	//branch_instructions
	if (COUNTER_AVAILABLE(BR_INS) && COUNTER_AVAILABLE(BR_MSP))
	{
		USE_COUNTER(BR_INS);
		USE_COUNTER(BR_MSP);
		fprintf (fp, "branch_instructions.overall = (PAPI_BR_INS * BR_lat + PAPI_BR_MSP * BR_miss_lat) / PAPI_TOT_INS\n");
		fprintf (fp, "branch_instructions.correctly_predicted = PAPI_BR_INS * BR_lat / PAPI_TOT_INS\n");
		fprintf (fp, "branch_instructions.mispredicted = PAPI_BR_MSP * BR_miss_lat / PAPI_TOT_INS\n\n");
	}
	else
		counter_err("branch_instructions");

	//floating-point_instr
	char* lcpi_slow = NULL;
	char* lcpi_fast = NULL;

	if (COUNTER_AVAILABLE(FML_INS))
	{
		USE_COUNTER(FML_INS);
		if (COUNTER_AVAILABLE(FAD_INS))
		{
			USE_COUNTER(FAD_INS);
			lcpi_fast = "((PAPI_FML_INS + PAPI_FAD_INS) * FP_lat)";
		}
		else
			lcpi_fast = "(PAPI_FML_INS * FP_lat)";
	}
	else if (COUNTER_AVAILABLE(FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION) && COUNTER_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP) && COUNTER_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP_PACKED) && COUNTER_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP_SCALAR) && COUNTER_AVAILABLE(FP_COMP_OPS_EXE_SSE_SINGLE_PRECISION) && COUNTER_AVAILABLE(FP_COMP_OPS_EXE_X87))
	{
		USE_COUNTER(FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION_SSE_FP_SSE_FP_PACKED_SSE_FP_SCALAR_SSE_SINGLE_PRECISION_X87);
		DELETE_COUNTER(FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION);
		DELETE_COUNTER(FP_COMP_OPS_EXE_SSE_FP);
		DELETE_COUNTER(FP_COMP_OPS_EXE_SSE_FP_PACKED);
		DELETE_COUNTER(FP_COMP_OPS_EXE_SSE_FP_SCALAR);
		DELETE_COUNTER(FP_COMP_OPS_EXE_SSE_SINGLE_PRECISION);
		DELETE_COUNTER(FP_COMP_OPS_EXE_X87);

		lcpi_fast = "(FP_COMP_OPS_EXE:SSE_DOUBLE_PRECISION:SSE_FP:SSE_FP_PACKED:SSE_FP_SCALAR:SSE_SINGLE_PRECISION:X87 * FP_lat)";
	}
	else	counter_err("floating-point_instr.fast_FP_inst");

	if (COUNTER_AVAILABLE(FDV_INS))
	{
		USE_COUNTER(FDV_INS);
		lcpi_slow = "(PAPI_FDV_INS * FP_slow_lat)";
	}
	else if (COUNTER_AVAILABLE(ARITH_CYCLES_DIV_BUSY))
	{
		USE_COUNTER(ARITH_CYCLES_DIV_BUSY);
		lcpi_slow = "(ARITH:CYCLES_DIV_BUSY * FP_slow_lat)";
	}
	else	counter_err("floating-point_instr.slow_FP_inst");

	fprintf (fp, "floating-point_instr.overall = (%s + %s) / PAPI_TOT_INS\n", lcpi_fast, lcpi_slow);
	fprintf (fp, "floating-point_instr.fast_FP_instr = %s / PAPI_TOT_INS\n", lcpi_fast);
	fprintf (fp, "floating-point_instr.slow_FP_instr = %s / PAPI_TOT_INS\n", lcpi_slow);
	fclose (fp);

	int event_code;
	int exp_count = 0;
	int papi_tot_ins_code;

	short remaining;
	PAPI_event_name_to_code("PAPI_TOT_INS", &papi_tot_ins_code);

	int rel_primes [4] = { 510007, 510009, 530003, 510013 };

	fp = fopen ("experiment.header", "w");
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

		remaining = 0;
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
					counter_present[j] = COUNTER_ADDED;
					if (remaining == 0)	// New line
						fprintf (fp, "experiment[%d]=\"", exp_count);

					if (j == TOT_INS)	fprintf (fp, "--event=%s:13000049 ", counter_names[j]);
					else			fprintf (fp, "--event=%s:%d ", counter_names[j], rel_primes[remaining%4]);
				}

				remaining++;

				// If we've used all the relatively prime numbers, break into the next experiment
				if (remaining == 3)
					break;
			}
		}

		exp_count++;
		if (remaining > 0)	fprintf (fp, "--event=PAPI_TOT_INS:13000027\"\n");
	} while (remaining > 0);

	fclose(fp);

	fprintf (stderr, "Generated LCPI and experiment files\n");
	return 0;
}
