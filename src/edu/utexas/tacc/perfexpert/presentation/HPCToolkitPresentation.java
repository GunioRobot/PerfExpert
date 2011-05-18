/*
    Copyright (C) 2011 The University of Texas at Austin

    This file is part of PerfExpert.

    PerfExpert is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PerfExpert is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with PerfExpert.  If not, see <http://www.gnu.org/licenses/>.

    Author: Ashay Rane
*/

package edu.utexas.tacc.perfexpert.presentation;

import java.text.DecimalFormat;
import java.util.List;
import java.util.Map;

import org.apache.log4j.Logger;

import edu.utexas.tacc.perfexpert.configuration.LCPIConfigManager;
import edu.utexas.tacc.perfexpert.configuration.MachineConfigManager;
import edu.utexas.tacc.perfexpert.configuration.hpctoolkit.mathparser.MathParser;
import edu.utexas.tacc.perfexpert.configuration.hpctoolkit.mathparser.ParseException;
import edu.utexas.tacc.perfexpert.parsing.profiles.hpctoolkit.HPCToolkitProfile;
import edu.utexas.tacc.perfexpert.parsing.profiles.hpctoolkit.HPCToolkitProfileConstants;

public class HPCToolkitPresentation
{
	static short maxBarWidth = 47;
	enum Metric { METRIC_RATIO, METRIC_LCPI };
	static Logger log = Logger.getLogger( HPCToolkitPresentation.class );
	public static void presentSummaryProfiles(List<HPCToolkitProfile> profiles01, List<HPCToolkitProfile> profiles02, LCPIConfigManager lcpiConfig, MachineConfigManager machineConfig, String file01, String file02, boolean aggregateOnly)
	{
		Metric metricType;

		if (profiles01 == null || profiles01.size() == 0)
		{
			log.error("Received empty profiles as input, terminating...");
			return;
		}

		if (lcpiConfig == null)
		{
			log.error("Received empty LCPI configuration as input, terminating...");
			return;
		}

		if (machineConfig == null)
		{
			log.error("Received empty machine configuration as input, terminating...");
			return;
		}

		DecimalFormat doubleFormat = new DecimalFormat("#.###");
		MathParser mathParser = new MathParser();
		HPCToolkitProfileConstants profileConstants = profiles01.get(0).getConstants();
		Map<String, Integer> perfCtrTranslation = profileConstants.getPerfCounterTranslation();
		Map<String, Integer> lcpiTranslation = profileConstants.getLCPITranslation();
		
		Integer indexOfCycles = perfCtrTranslation.get("PAPI_TOT_CYC");
		int indexOfInstructions = profileConstants.getIndexOfInstructions();
		String CPIThreshold = machineConfig.getProperties().getProperty("CPI_threshold");
		
		if (indexOfCycles == null)
		{
			log.error("Could not find PAPI_TOT_CYC among the list of discovered counters, cannot proceed with LCPI computation");
			return;
		}
		
		if (CPIThreshold == null)
		{
			log.error("Could not find CPI_threshold in machine.properties, defaulting to 0.5");
			CPIThreshold = "0.5";
		}

		long cpuFrequency = Long.parseLong(machineConfig.getProperties().getProperty("CPU_freq"));

		for (HPCToolkitProfile profile : profiles01)
		{
			if (profile.getCodeSectionInfo().equals("Aggregate"))
			{
				System.out.println ("Total running time for \"" + file01 + "\" is " + doubleFormat.format(profile.getMetricBasedOnPEIndex(indexOfCycles)/cpuFrequency) + " sec");
				break;
			}
		}

		if (profiles02 != null)
		{
			for (HPCToolkitProfile profile : profiles02)
			{
				if (profile.getCodeSectionInfo().equals("Aggregate"))
				{
					Integer indexOfCycles2 = profile.getConstants().getPerfCounterTranslation().get("PAPI_TOT_CYC");
					System.out.println ("Total running time for \"" + file02 + "\" is " + doubleFormat.format(profile.getMetricBasedOnPEIndex(indexOfCycles2)/cpuFrequency) + " sec");
					break;
				}
			}
		}


		double dCPIThreshold = Double.parseDouble(CPIThreshold);
		for (HPCToolkitProfile profile : profiles01)
		{
			if ((profile.getCodeSectionInfo().equals("Aggregate") && !aggregateOnly) ||
				(aggregateOnly && !profile.getCodeSectionInfo().equals("Aggregate")))
				continue;

			if (profile.getImportance() == -1)
				continue;

			double cycles = profile.getMetricBasedOnPEIndex(indexOfCycles);			
			double instructions = profile.getMetricBasedOnPEIndex(indexOfInstructions);

			HPCToolkitProfile matchingProfile = getMatchingProfile(profile, profiles02);

			if (matchingProfile == null)
				System.out.println("\n" + profile.getCodeSectionInfo() + " (" + doubleFormat.format(profile.getImportance()*100) + "% of the total runtime)");
			else
			{
				Integer indexOfCycles2 = profiles02.get(0).getConstants().getPerfCounterTranslation().get("PAPI_TOT_CYC");
				double cycles2 = matchingProfile.getMetricBasedOnPEIndex(indexOfCycles2);

				System.out.println("\n" + profile.getCodeSectionInfo() + " (runtimes are " + doubleFormat.format(cycles/cpuFrequency) + "s and " + doubleFormat.format(cycles2/cpuFrequency) + "s)");
			}

			System.out.println("===============================================================================");

			double maxVariation = matchingProfile == null ? profile.getVariation() : (profile.getVariation() > matchingProfile.getVariation() ? profile.getVariation() : matchingProfile.getVariation());
			if (maxVariation > 0.2)
				System.out.println("WARNING: The instruction count variation is " + doubleFormat.format(maxVariation*100) + "%, making the results unreliable");

			if (cycles < cpuFrequency)
			{
				System.out.println("WARNING: The runtime for this code section is too short to gather meaningful measurements");
				continue;
			}

			double cpi = cycles / instructions;
			if (cpi <= dCPIThreshold)
				System.out.println("The performance of this code section is good");

			boolean printRatioHeader = false, printPerfHeader = false;
			// Compute each LCPI metric
			for (String LCPI : lcpiConfig.getLCPINames())
			{
				String category, subcategory;
				int indexOfPeriod = LCPI.indexOf(".");
				if (indexOfPeriod < 0)
				{
					category = "overall";
					subcategory = LCPI;
				}
				else
				{
					category = LCPI.substring(0, indexOfPeriod);
					subcategory = LCPI.substring(indexOfPeriod+1);
				}

				String formula = lcpiConfig.getProperties().getProperty(LCPI);
				int index = lcpiTranslation.get(LCPI);

				double result01 = 0;
				try
				{
					result01 = Double.valueOf(doubleFormat.format(mathParser.parse(formula, profile, machineConfig.getProperties())));
				}
				catch (ParseException e)
				{
					log.error("Error in parsing expression: " + formula + "\nDefaulting value of " + LCPI + " to zero.\n[" + e.getMessage() + "]\n" + e.getStackTrace());
				}

				profile.setLCPI(index, result01);
				log.debug(profile.getCodeSectionInfo() + ": " + category + "." + subcategory + " = " + result01);

				double result02 = result01;
				if (matchingProfile != null)
				{
					try
					{
						result02 = Double.valueOf(doubleFormat.format(mathParser.parse(formula, matchingProfile, machineConfig.getProperties())));
					}
					catch (ParseException e)
					{
						log.error("Error in parsing expression: " + formula + "\nDefaulting value of " + LCPI + " to zero.\n[" + e.getMessage() + "]\n" + e.getStackTrace());
					}
	
					matchingProfile.setLCPI(index, result02);
					log.debug(matchingProfile.getCodeSectionInfo() + ": " + category + "." + subcategory + " = " + result02);
				}

				String fCategory = category.replaceAll("_", " ");
				String fSubcategory = subcategory.replaceAll("_", " ");

				if (category.regionMatches(true, 0, "ratio", 0, category.length()))
				{
					metricType = Metric.METRIC_RATIO;
					if (printRatioHeader == false)
					{
						// Print ratio header
						System.out.println (String.format("%-" + (lcpiConfig.getLargestLCPINameLength()+4) + "s    %%  0.........25...........50.........75........100", "ratio to total instrns"));
						printRatioHeader = true;
					}
				}
				else
				{
					metricType = Metric.METRIC_LCPI;
					if (printPerfHeader == false)
					{
						// Print perf header
						System.out.println("\n-------------------------------------------------------------------------------");
						System.out.println (String.format("%-" + (lcpiConfig.getLargestLCPINameLength()+4) + "s  LCPI good......okay......fair......poor......bad....", "performance assessment"));
						printPerfHeader = true;
					}
				}

				if (subcategory.regionMatches(true, 0, "overall", 0, subcategory.length()))
				{
					// Print the category name
					System.out.print(String.format("%-" + (lcpiConfig.getLargestLCPINameLength()+4) + "s: ", "* " + fCategory));
				}
				else
					System.out.print(String.format("%-" + (lcpiConfig.getLargestLCPINameLength()+4) + "s: ", "   - " + fSubcategory));

				if (matchingProfile == null)
				{
					if (metricType == Metric.METRIC_RATIO)
					{
						// FIXME: L1_DCA / TOT_INS ration going beyond 100% in some cases

						// Cap values to 100
						if (result01 > 1)	result01 = 1;
						if (result02 > 1)	result02 = 1;

						System.out.print(String.format("%4.0f ", result01*100));
						printRatioBar(result01*100, result02*100, dCPIThreshold);
					}
					else
					{
						System.out.print(String.format("%4.1f ", result01));
						printLCPIBar(result01, result02, dCPIThreshold);
					}
				}
				else
				{
					if (metricType == Metric.METRIC_RATIO)
					{
						// Cap values to 100
						if (result01 > 1)	result01 = 1;
						if (result02 > 1)	result02 = 1;

						System.out.print(String.format("     ", result01*100));
						printRatioBar(result01*100, result02*100, dCPIThreshold);
					}
					else
					{
						System.out.print("     ");
						printLCPIBar(result01, result02, dCPIThreshold);
					}
				}

				if (category.regionMatches(true, 0, "overall", 0, category.length()))
				{
					// Print line about upper bounds
					System.out.println("upper bound estimates");
				}
			}
		}
	}

	private static HPCToolkitProfile getMatchingProfile(HPCToolkitProfile needle, List<HPCToolkitProfile> smallHaystack)
	{
		if (smallHaystack == null)
			return null;

		String searchString = needle.getCodeSectionInfo();
		for (HPCToolkitProfile profile : smallHaystack)
			if (profile.getCodeSectionInfo().equals(searchString))
				return profile;
		
		return null;
	}

	private static void printRatioBar(double value1, double value2, double cpiThreshold)
	{
		// Scale to maxBarWidth
		value1 *= maxBarWidth / 100.0;
		value2 *= maxBarWidth / 100.0;

		if (value1 < 1)	value1 = 1;
		if (value2 < 1)	value2 = 1;

		char term = ' ';
		if (value1 > maxBarWidth)
		{
			term = '+';
			value1 = maxBarWidth-1;
		}

		if (value2 > maxBarWidth)
		{
			term = '+';
			value2 = maxBarWidth-1;
		}

		double min = value1 < value2 ? value1 : value2;
		value1 -= min;
		value2 -= min;

		while(min-- > 0.5)
			System.out.print('*');

		if (value1 > 0)
		{
			value1 += min;
			while (value1-- > 0.5)
				System.out.print(1);
		}
		else
		{
			value2 += min;
			while (value2-- > 0.5)
				System.out.print(2);
		}

		if (term != ' ')
			System.out.print(term);

		System.out.print("\n");
	}

	private static void printLCPIBar(double value1, double value2, double cpiThreshold)
	{
		value1 *= 10 / cpiThreshold;
		value2 *= 10 / cpiThreshold;
		
		if (value1 < 1)	value1 = 1;
		if (value2 < 1)	value2 = 1;

		char term = ' ';
		if (value1 > maxBarWidth)
		{
			term = '+';
			value1 = maxBarWidth-1;
		}

		if (value2 > maxBarWidth)
		{
			term = '+';
			value2 = maxBarWidth-1;
		}

		double min = value1 < value2 ? value1 : value2;
		value1 -= min;
		value2 -= min;

		while(min-- > 0.5)
			System.out.print('>');

		if (value1 > 0)
		{
			value1 += min;
			while (value1-- > 0.5)
				System.out.print(1);
		}
		else
		{
			value2 += min;
			while (value2-- > 0.5)
				System.out.print(2);
		}

		if (term != ' ')
			System.out.print(term);

		System.out.print("\n");
	}
}
