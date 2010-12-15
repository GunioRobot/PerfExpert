package edu.utexas.tacc.perfexpert.configuration;

public class LCPIConfigManager extends AConfigManager
{
	public LCPIConfigManager(String sourceURI)
	{
		super(sourceURI);
	}

	public String [] getLCPINames()
	{
		// Size-1 because we do not want the "version" string
		String [] names = new String [props.size()-1];

		int ctr=0;
		for (Object key : props.keySet())
		{
			String sKey = (String) key;
			if (!sKey.equals("version"))
				names[ctr++] = (String) key;
		}
		
		return names;
	}

	// Nothing *really* special about this configuration manager
}
