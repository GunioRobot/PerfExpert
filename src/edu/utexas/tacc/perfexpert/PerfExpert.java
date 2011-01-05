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
		
		if (args.length < 2 || args.length > 3)
		{
			System.out.println("USAGE: PerfExpert threshold first-input-file [second-input-file]");
			System.exit(1);
		}

		Double threshold = 0.1; 
		try
		{
			threshold = Double.parseDouble(args[0]);
		} catch (NumberFormatException e)
		{
			log.error("Threshold parameter was invalid, expected a double, instead found \"" + args[0] + "\"");
			return;
		}

		if (threshold == null || threshold < 0 || threshold > 1.0)
		{
			log.error("Threshold parameter was invalid, expected a double between 0 and 1, instead found \"" + args[0] + "\"");
			return;
		}

		String filename01 = args[1];
		if (filename01 == null || filename01.isEmpty())
		{
			log.error("Invalid filename passed as input \"" + filename01 + "\"");
			return;
		}

		String filename02 = null;
		if (args.length == 3)
		{
			filename02 = args[2];
			if (filename02 == null || filename02.isEmpty())
			{
				log.error("Invalid filename passed as input \"" + filename02 + "\"");
				return;
			}
		}

		log.debug("PerfExpert invoked with arguments: " + threshold + ", " + filename01 + ", " + filename02);

		String peConfigLocation = "/opt/apps/perfexpert";
		if (!new File(peConfigLocation + "/perfexpert.properties").exists())
		{
			// Try home dir
			peConfigLocation = System.getProperty("user.home") + "/.perfexpert";
			if (!new File(peConfigLocation + "/perfexpert.properties").exists())
			{
				// Could not find in the alternate location also, give up
				log.error("Could not find seed configuration file perfexpert.properties in either /opt/apps/perfexpert or " + peConfigLocation + ", terminating...");
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
		
		List<HPCToolkitProfile> profiles01 = null, profiles02 = null;

		HPCToolkitParser parser01 = new HPCToolkitParser(HPCDATA_LOCATION, threshold, "file://" + filename01, lcpiConfig.getProperties());
		profiles01 = parser01.parse();

		if (filename02 != null)
		{
			HPCToolkitParser parser02 = new HPCToolkitParser(HPCDATA_LOCATION, 0, "file://" + filename02, lcpiConfig.getProperties());
			profiles02 = parser02.parse();
		}

		HPCToolkitPresentation.presentSummaryProfiles(profiles01, profiles02, lcpiConfig, machineConfig);
	}
}
