package edu.utexas.tacc.perfexpert;

import org.apache.log4j.Logger;

import edu.utexas.tacc.perfexpert.parsing.hpctoolkit.HPCToolkitParser;
import edu.utexas.tacc.perfexpert.parsing.profiles.hpctoolkit.HPCToolkitProfile;

public class PerfExpert
{
	public static void main(String[] args) throws Exception
	{
		Logger log = Logger.getLogger( PerfExpert.class );

		HPCToolkitParser parser = new HPCToolkitParser(0.1, "file:///home/ashay/temp/perfexpert/experiment.xml");
		for (HPCToolkitProfile profile : parser.parse())
			log.info(profile.getCodeSectionInfo() + " - " + profile.getImportance());
	}
}
