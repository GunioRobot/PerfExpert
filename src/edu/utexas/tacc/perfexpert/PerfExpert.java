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

package edu.utexas.tacc.perfexpert;

import java.io.File;
import java.util.List;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

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

		String PERFEXPERT_HOME="";
		ClassLoader loader = PerfExpert.class.getClassLoader();
		String regex = "jar:file:(.*)/bin/perfexpert.jar!/edu/utexas/tacc/perfexpert/PerfExpert.class";
		String jarURL = loader.getResource("edu/utexas/tacc/perfexpert/PerfExpert.class").toString();

		Pattern p = Pattern.compile(regex);
		Matcher m = p.matcher(jarURL);

		if (m.find())
			PERFEXPERT_HOME = m.group(1);
		else
		{
			log.error("Could not extract location of PerfExpert jar from URL: (" + jarURL + "), was using the regex: \"" + regex + "\"");
			System.exit(1);
		}

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

		String HPCDATA_LOCATION = System.getenv("HPCDATA_HOME");
		if (HPCDATA_LOCATION == null || HPCDATA_LOCATION.isEmpty())
		{
			log.error("The environment variable ${HPCDATA_HOME} was not set, cannot proceed");
			return;
		}

		LCPIConfigManager lcpiConfig = new LCPIConfigManager("file://" + PERFEXPERT_HOME + "/config/lcpi.properties");
		if (lcpiConfig.readConfigSource() == false)		// Error while reading configuraiton, handled inside method
			return;
		
		MachineConfigManager machineConfig = new MachineConfigManager("file://" + PERFEXPERT_HOME + "/config/machine.properties");
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
