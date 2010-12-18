package edu.utexas.tacc.perfexpert.parsing.profiles.hpctoolkit;

import org.apache.log4j.Logger;

import edu.utexas.tacc.perfexpert.parsing.profiles.AProfile;

public class HPCToolkitProfile extends AProfile implements Comparable<HPCToolkitProfile>
{
	Logger log = Logger.getLogger( HPCToolkitProfile.class );

	private HPCToolkitProfileConstants profileConstants;
	
	int LCPICount = 10;

	// Used for measuring the amount of variation
	double maxCycles = Double.MIN_VALUE, minCycles = Double.MAX_VALUE;

	double importance = -1;
	double [] perfValues;
	double [] lcpiValues;
	int [] counts;	// For averages
	
	public HPCToolkitProfile(HPCToolkitProfileConstants profileConstants)
	{
		this.profileConstants = profileConstants;
		perfValues = new double [profileConstants.discoveredMetrics];
		counts = new int [profileConstants.discoveredMetrics];
		lcpiValues = new double [LCPICount];
	}
	
	public void setLCPI(int index, double value)
	{
		if (index > LCPICount)
			return;

		lcpiValues[index] = value;
	}

	public double getLCPI(int index)
	{
		if (index > LCPICount)
			return 0;

		return lcpiValues[index];
	}

	@Override
	public double getMetric(String key)
	{
		Integer PEIndex = profileConstants.perfCounterTranslation.get(key);
		if (PEIndex == null)
		{
			// The metric does not exist
			return 0;
		}

		return getMetricBasedOnPEIndex(PEIndex);
	}

	public double getMetric(int HPCToolkitIndex)
	{
		int PEIndex = profileConstants.HPCToPETranslation.get(HPCToolkitIndex);
		return getMetricBasedOnPEIndex(PEIndex);
	}
	
	public double getMetricBasedOnPEIndex(int PEIndex)
	{
		if (counts[PEIndex] == 0 || perfValues[PEIndex] == 0)
			return 0;
		
		return perfValues[PEIndex]/counts [PEIndex];
	}

	@Override
	public void setMetric(String key, double value)
	{
		int peIndex = profileConstants.perfCounterTranslation.get(key);
		setMetricBasedOnPEIndex(peIndex, value);
	}

	public void setMetric(int HPCToolkitIndex, double value)
	{
		int peIndex = profileConstants.HPCToPETranslation.get(HPCToolkitIndex);
		setMetricBasedOnPEIndex(peIndex, value);
	}
	
	private void setMetricBasedOnPEIndex(int PEIndex, double value)
	{
		perfValues[PEIndex] += value;
		counts[PEIndex]++;
		
		if (PEIndex == profileConstants.indexOfCycles)
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

@Override
	public int compareTo(HPCToolkitProfile o)
	{
		return (int) (importance - o.getImportance()); 
	}
}
