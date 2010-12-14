package edu.utexas.tacc.perfexpert;

import org.apache.log4j.Logger;

import edu.utexas.tacc.perfexpert.configuration.MachineConfigManager;

public class PerfExpert
{
	public static void main(String[] args) throws Exception
	{
		Logger log = Logger.getLogger( PerfExpert.class );
		log.info("Hello, World!");
		
		MachineConfigManager machineConfig = new MachineConfigManager();
	}
}
