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
	static Logger log = Logger.getLogger( HPCToolkitPresentation.class );
	public static void presentSummaryProfiles(List<HPCToolkitProfile> profiles01, List<HPCToolkitProfile> profiles02, LCPIConfigManager lcpiConfig, MachineConfigManager machineConfig)
	{
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
		
		int indexOfCycles = profileConstants.getIndexOfCycles();
		Integer indexOfInstructions = perfCtrTranslation.get("PAPI_TOT_INS");
		String CPIThreshold = machineConfig.getProperties().getProperty("CPI_threshold");
		
		if (indexOfInstructions == null)
		{
			log.error("Could not find PAPI_TOT_INS among the list of discovered counters, cannot proceed with LCPI computation");
			return;
		}
		
		if (CPIThreshold == null)
		{
			log.error("Could not find CPI_threshold in machine.properties, defaulting to 0.5");
			CPIThreshold = "0.5";
		}
		
		double dCPIThreshold = Double.parseDouble(CPIThreshold);
		for (HPCToolkitProfile profile : profiles01)
		{
			if (profile.getCodeSectionInfo().equals("Aggregate") || profile.getImportance() == -1)
				continue;

			long cpuFrequency = Long.parseLong(machineConfig.getProperties().getProperty("CPU_freq"));
			double cycles = profile.getMetricBasedOnPEIndex(indexOfCycles);			
			double instructions = profile.getMetricBasedOnPEIndex(indexOfInstructions);

			HPCToolkitProfile matchingProfile = getMatchingProfile(profile, profiles02);

			if (matchingProfile == null)
				System.out.println("\n" + profile.getCodeSectionInfo() + " (" + (profile.getImportance()*100) + "% of the total runtime)");
			else
			{
				double cycles2 = matchingProfile.getMetricBasedOnPEIndex(indexOfCycles);
				System.out.println("\n" + profile.getCodeSectionInfo() + " (runtimes are " + doubleFormat.format(cycles/cpuFrequency) + "s and " + doubleFormat.format(cycles2/cpuFrequency) + "s)");
			}

			System.out.println("--------------------------------------------------------------------------------");

			double maxVariation = matchingProfile == null ? profile.getVariation() : (profile.getVariation() > matchingProfile.getVariation() ? profile.getVariation() : matchingProfile.getVariation());
			if (maxVariation > 0.2)
				System.out.println("WARNING: The cycle count variation is " + doubleFormat.format(maxVariation*100) + "%, making the results unreliable");

			if (cycles < cpuFrequency)
			{
				log.warn("WARNING: The runtime for " + profile.getCodeSectionInfo() + " is too short to gather meaningful measurements");
				continue;
			}

			double cpi = cycles / instructions;
			if (cpi <= dCPIThreshold)
				System.out.println("The performance of this code section is good");
			else
			{
				System.out.println("performance assessment    LCPI good......okay......fair......poor......bad....");
				// Compute each LCPI metric
				for (String LCPI : lcpiConfig.getLCPINames())
				{
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
					log.debug(profile.getCodeSectionInfo() + ": " + LCPI + " = " + result01);
					
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
						log.debug(matchingProfile.getCodeSectionInfo() + ": " + LCPI + " = " + result02);
					}
					
					if (matchingProfile == null)
						System.out.print(String.format("%23s: %4.1f  ", LCPI, result01));
					else
						System.out.print(String.format("%23s:       ", LCPI));

					printBar(result01, result02, dCPIThreshold);
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
	
	private static void printBar(double value1, double value2, double cpiThreshold)
	{
		value1 *= 5 / cpiThreshold;
		value2 *= 5 / cpiThreshold;
		
		if (value1 < 1)	value1 = 1;
		if (value2 < 1)	value2 = 1;

		char term = ' ';
		if (value1 > 47)
		{
			term = '+';
			value1 = 46;
		}

		if (value2 > 47)
		{
			term = '+';
			value2 = 46;
		}

		double min = value1 < value2 ? value1 : value2;
		value1 -= min;
		value2 -= min;
		
		while(min-- > 0.5)
			System.out.print(">");

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
