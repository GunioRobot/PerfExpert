package edu.utexas.tacc.perfexpert.configuration;

public class LCPIConfigManager extends AConfigManager
{
	int largestStringLength = -1;
	public LCPIConfigManager(String sourceURI)
	{
		super(sourceURI);
	}

	public final String [] getLCPINames()
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
	
	public final int getLargestLCPINameLength()
	{
		if (largestStringLength != -1)
			return largestStringLength;

		int max = -1;
		int checkValue = -1;
		for (Object key : props.keySet())
		{
			String sKey = (String) key;
			checkValue = sKey.indexOf(".");
			checkValue = (checkValue == -1 ? sKey.length() : sKey.length()-checkValue);
			if (max < checkValue)
				max = checkValue;
		}

		largestStringLength = max;
		return max;
	}

	// Nothing *really* special about this configuration manager
}
