package edu.utexas.tacc.perfexpert.parsing.profiles.hpctoolkit;

import org.apache.log4j.Logger;

import edu.utexas.tacc.perfexpert.parsing.profiles.AProfile;

public class HPCToolkitProfile extends AProfile
{
	Logger log = Logger.getLogger( HPCToolkitProfile.class );

	private HPCToolkitProfileConstants profileConstants;

	// Used for measuring the amount of variation
	double maxCycles = Double.MIN_VALUE, minCycles = Double.MAX_VALUE;

	double importance = -1;
	double [] perfValues;
	double [] lcpiValues;
	
	public HPCToolkitProfile(HPCToolkitProfileConstants profileConstants)
	{
		this.profileConstants = profileConstants;
		perfValues = new double [profileConstants.discoveredMetrics];
	}

	@Override
	public double getMetric(String key)
	{
		return perfValues[profileConstants.perfCounterTranslation.get(key)];
	}

	public double getMetric(int HPCToolkitIndex)
	{
		return perfValues[profileConstants.HPCToPETranslation.get(HPCToolkitIndex)];
	}

	@Override
	public void setMetric(String key, double value)
	{
		int peIndex = profileConstants.perfCounterTranslation.get(key);
		perfValues[peIndex] = value;
		
		if (peIndex == profileConstants.indexOfCycles)
		{
			// Adjust min and max
			if (minCycles > value)	minCycles = value;
			if (maxCycles < value)	maxCycles = value;

			// Set importance
			if (profileConstants.aggregateCycles != 0)
				importance = value / ((double) profileConstants.aggregateCycles);
			else
			{
				// If everything is correct, this is the record of the aggregate execution
				// Unfortunately, there is no way to distinguish within this limited context
				// So as a safe measure, we just ignore, which is what we would do if everything were right!
			}
		}
	}

	public void setMetric(int HPCToolkitIndex, double value)
	{
		int peIndex = profileConstants.HPCToPETranslation.get(HPCToolkitIndex);
		perfValues[peIndex] = value;
		
		if (peIndex == profileConstants.indexOfCycles)
		{
			// Adjust min and max
			if (minCycles > value)	minCycles = value;
			if (maxCycles < value)	maxCycles = value;

			// Set importance
			if (profileConstants.aggregateCycles != 0)
				importance = value / ((double) profileConstants.aggregateCycles);
			else
			{
				// If everything is correct, this is the record of the aggregate execution
				// Unfortunately, there is no way to distinguish within this limited context
				// So as a safe measure, we just ignore, which is what we would do if everything were right!
			}
		}
	}
	
	public HPCToolkitProfileConstants getConstants()
	{
		return profileConstants;
	}
	
	public double getVariation()
	{
		if (maxCycles == Double.MIN_VALUE)	// Value was never set
			return 0;

		return maxCycles-minCycles;
	}
	
	public double getImportance ()
	{
		return importance;
	}
	
	public void setImportance(double importance)
	{
		this.importance = importance;
	}
}
