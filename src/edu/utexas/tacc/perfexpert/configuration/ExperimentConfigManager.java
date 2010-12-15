package edu.utexas.tacc.perfexpert.configuration;

import java.util.ArrayList;
import java.util.LinkedHashSet;

public class ExperimentConfigManager extends AConfigManager
{
	public ExperimentConfigManager(String sourceURI)
	{
		super(sourceURI);
	}
	
	public ArrayList<String> getPerfCounterList()
	{
		// Use a set, then convert to an ArrayList
		// We use an ArrayList so that it is possible to get the index of the element
		
		// Constructing with an arbitrary initial capacity of 16 
		LinkedHashSet<String> set = new LinkedHashSet<String>(16);
		for (Object key : props.keySet())
		{
			String sKey = (String) key;
			if (sKey.startsWith("PAPI_"))
				set.add(sKey);
		}

		if (set.size() == 0)
			log.debug("Zero performance counters loaded from " + sourceURI + ". Is this expected?");

		// Converting to ArrayList
		return new ArrayList<String>(set);
	}
	
	// Returns a list with strings of the form: -e PAPI_TOT_CYC:13343242 -e PAPI_TOT_INS:3454353 ...
	public String [] getExperimentParameters()
	{
		int experiments = 0;
		for (Object key : props.keySet())
		{
			String sKey = (String) key;
			if (sKey.startsWith("experiment."))
				experiments++;
		}

		String [] params = new String [experiments];
		for (Object key : props.keySet())
		{
			String sKey = (String) key;
			if (sKey.startsWith("experiment."))
			{
				// Get the number in the second half of the key
				int expNumber = Integer.parseInt(sKey.split("\\.")[1]);
				String value = props.getProperty(sKey);

				// Get the formatted string with "-e" before each counter:frequency pair
				String expParam = "";
				for (String counter : value.split(","))
					expParam += " -e " + counter + ":" + props.getProperty(counter, "0");

				log.debug("Adding new experiment parameter (" + (expNumber-1) + ") to list: \n" + expParam);
				if (expNumber > 0)
					params [expNumber-1] = expParam;
			}
		}

		if (params.length == 0)
			log.debug("Resulting configuration loaded from " + sourceURI + " has zero entries. Is this expected?");

		return params;
	}

	public int getSamplingFrequency(String perfCounter)
	{
		if (perfCounter.startsWith("PAPI_"))
			return Integer.parseInt(props.getProperty(perfCounter));
		
		return 0;
	}
}
