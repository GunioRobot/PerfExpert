package edu.utexas.tacc.perfexpert;

import java.io.File;
import java.util.List;

import org.apache.log4j.Logger;

import edu.utexas.tacc.perfexpert.configuration.LCPIConfigManager;
import edu.utexas.tacc.perfexpert.configuration.MachineConfigManager;
import edu.utexas.tacc.perfexpert.configuration.PerfExpertConfigManager;
import edu.utexas.tacc.perfexpert.parsing.hpctoolkit.HPCToolkitParser;
import edu.utexas.tacc.perfexpert.parsing.profiles.hpctoolkit.HPCToolkitProfile;
import edu.utexas.tacc.perfexpert.presentation.HPCToolkitPresentation;

public class PerfExpert
{
	public static void main(String[] args) throws Exception
	{
		Logger log = Logger.getLogger( PerfExpert.class );
		
		String peConfigLocation = "/opt/apps/perfexpert";
		if (!new File(peConfigLocation + "/perfexpert.properties").exists())
		{
			// Try home dir
			peConfigLocation = System.getProperty("user.home") + "/.perfexpert";
			if (!new File(peConfigLocation + "/perfexpert.properties").exists())
			{
				// Could not find in the alternate location also, give up
				log.error("Could not find seed configuration file perfexpert.properties, terminating...");
				return;
			}
		}
		
		PerfExpertConfigManager peConfig = new PerfExpertConfigManager("file://" + peConfigLocation + "/perfexpert.properties");
		peConfig.readConfigSource();

		String CONFIG_LOCATION = peConfig.getProperties().getProperty("CONFIG_LOCATION");
		if (CONFIG_LOCATION == null || CONFIG_LOCATION.isEmpty())
		{
			log.error("CONFIG_LOCATION was not set in " + peConfigLocation + "/perfexpert.properties, cannot proceed");
			return;
		}

		String HPCDATA_LOCATION = peConfig.getProperties().getProperty("HPCDATA_LOCATION");
		if (HPCDATA_LOCATION == null || HPCDATA_LOCATION.isEmpty())
		{
			log.error("HPCDATA_LOCATION was not set in " + peConfigLocation + "/perfexpert.properties, cannot proceed");
			return;
		}

		LCPIConfigManager lcpiConfig = new LCPIConfigManager("file://" + CONFIG_LOCATION + "/lcpi.properties");
		if (lcpiConfig.readConfigSource() == false)		// Error while reading configuraiton, handled inside method
			return;
		
		MachineConfigManager machineConfig = new MachineConfigManager("file://" + CONFIG_LOCATION + "/machine.properties");
		if (machineConfig.readConfigSource() == false)		// Error while reading configuraiton, handled inside method
			return;
		
		HPCToolkitParser parser01 = new HPCToolkitParser(HPCDATA_LOCATION, 0.1, "file:///home/klaus/temp/perfexpert/sample-perfexpert-files/01.xml", lcpiConfig.getProperties());
		List<HPCToolkitProfile> profiles01 = parser01.parse();

		HPCToolkitParser parser02 = new HPCToolkitParser(HPCDATA_LOCATION, 0.1, "file:///home/klaus/temp/perfexpert/sample-perfexpert-files/01-diff.xml", lcpiConfig.getProperties());
		List<HPCToolkitProfile> profiles02 = parser02.parse();

		HPCToolkitPresentation.presentSummaryProfiles(profiles01, profiles02, lcpiConfig, machineConfig);
	}
}
