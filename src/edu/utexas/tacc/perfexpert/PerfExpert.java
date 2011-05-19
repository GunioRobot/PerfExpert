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

import com.martiansoftware.jsap.JSAP;
import com.martiansoftware.jsap.Switch;
import com.martiansoftware.jsap.JSAPResult;
import com.martiansoftware.jsap.FlaggedOption;
import com.martiansoftware.jsap.UnflaggedOption;
import com.martiansoftware.jsap.stringparsers.DoubleStringParser;
import com.martiansoftware.jsap.stringparsers.StringStringParser;

import edu.utexas.tacc.perfexpert.misc.RangeParser;
import edu.utexas.tacc.perfexpert.configuration.LCPIConfigManager;
import edu.utexas.tacc.perfexpert.configuration.MachineConfigManager;
import edu.utexas.tacc.perfexpert.configuration.PerfExpertConfigManager;
import edu.utexas.tacc.perfexpert.parsing.hpctoolkit.HPCToolkitParser;
import edu.utexas.tacc.perfexpert.parsing.profiles.hpctoolkit.HPCToolkitProfile;
import edu.utexas.tacc.perfexpert.presentation.HPCToolkitPresentation;

public class PerfExpert
{
	public static void printHelp(JSAP parser)
	{
		System.out.println ("\nUSAGE: perfexpert " + parser.getUsage() + "\n");
		System.out.println (parser.getHelp());
	}

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

		JSAP parser = new JSAP();
		Switch helpSwitch = new Switch("help", 'h', "help", "Show this help screen");
		Switch aggregateSwitch = new Switch("aggregate", 'a', "aggregate", "Show whole-program information only, no function- or loop-level information");

		FlaggedOption threadParam = new FlaggedOption("threads", StringStringParser.getParser(), null, false, 't', "threads");
		UnflaggedOption thresholdParam = new UnflaggedOption("threshold", DoubleStringParser.getParser(), true, "Threshold between 0 and 1");
		UnflaggedOption xml1 = new UnflaggedOption("experiment1.xml", StringStringParser.getParser(), true, "experiment.xml file generated using `perfexpert_run_exp'");
		UnflaggedOption xml2 = new UnflaggedOption("experiment2.xml", StringStringParser.getParser(), false, "second experiment.xml file, for comparison only");

		xml1.setUsageName("experiment.xml");
		xml2.setUsageName("experiment.xml");

		parser.registerParameter(helpSwitch);
		parser.registerParameter(aggregateSwitch);
		parser.registerParameter(threadParam);
		parser.registerParameter(thresholdParam);
		parser.registerParameter(xml1);
		parser.registerParameter(xml2);

		JSAPResult result = parser.parse(args);

		if (!result.success() || result.getBoolean("help") || result.getDouble("threshold") < 0 || result.getDouble("threshold") > 1.0)
		{
			printHelp(parser);
			System.exit(0);
		}

		boolean aggregateOnly = result.getBoolean("aggregate");
		String threads = result.getString("threads");
		Double threshold = result.getDouble("threshold");
		String filename01 = result.getString("experiment1.xml");
		String filename02 = result.getString("experiment2.xml");

		log.debug("PerfExpert invoked with arguments: " + threshold + ", " + filename01 + ", " + filename02);

		String HPCDATA_LOCATION = System.getenv("PERFEXPERT_HPCDATA_HOME");
		if (HPCDATA_LOCATION == null || HPCDATA_LOCATION.length() == 0)
		{
			log.error("The environment variable ${PERFEXPERT_HPCDATA_HOME} was not set, cannot proceed");
			return;
		}

		LCPIConfigManager lcpiConfig = new LCPIConfigManager("file://" + PERFEXPERT_HOME + "/config/lcpi.properties");
		if (lcpiConfig.readConfigSource() == false)		// Error while reading configuraiton, handled inside method
			return;

		MachineConfigManager machineConfig = new MachineConfigManager("file://" + PERFEXPERT_HOME + "/config/machine.properties");
		if (machineConfig.readConfigSource() == false)		// Error while reading configuraiton, handled inside method
			return;

		String thread_regex = RangeParser.getRegexString(threads);

		List<HPCToolkitProfile> profiles01 = null, profiles02 = null;
		HPCToolkitParser parser01 = new HPCToolkitParser(HPCDATA_LOCATION, threshold, "file://" + filename01, lcpiConfig.getProperties());
		profiles01 = parser01.parse(aggregateOnly, thread_regex);

		if (filename02 != null)
		{
			HPCToolkitParser parser02 = new HPCToolkitParser(HPCDATA_LOCATION, 0, "file://" + filename02, lcpiConfig.getProperties());
			profiles02 = parser02.parse(aggregateOnly, thread_regex);
		}

		HPCToolkitPresentation.presentSummaryProfiles(profiles01, profiles02, lcpiConfig, machineConfig, filename01, filename02, aggregateOnly);
	}
}
