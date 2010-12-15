package edu.utexas.tacc.perfexpert;

import org.apache.log4j.Logger;
import edu.utexas.tacc.perfexpert.parsing.hpctoolkit.HPCToolkitParser;

public class PerfExpert
{
	public static void main(String[] args) throws Exception
	{
		Logger log = Logger.getLogger( PerfExpert.class );
		log.info("Hello, World!");

		HPCToolkitParser parser = new HPCToolkitParser("file:///home/klaus/temp/perfexpert/experiment.xml");
		parser.parse();
	}
}
