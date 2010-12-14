package edu.utexas.tacc.perfexpert;

import org.apache.log4j.Logger;

import edu.utexas.tacc.perfexpert.parsing.AParser;
import edu.utexas.tacc.perfexpert.parsing.HPCToolkitParser;

public class PerfExpert
{
	public static void main(String[] args) throws Exception
	{
		Logger log = Logger.getLogger( PerfExpert.class );
		
		HPCToolkitParser parser = new HPCToolkitParser("file:///tmp/experiment.xml");
	}
}
