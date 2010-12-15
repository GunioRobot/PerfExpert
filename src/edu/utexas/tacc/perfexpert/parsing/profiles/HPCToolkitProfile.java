package edu.utexas.tacc.perfexpert.parsing.profiles;

import java.util.HashMap;
import java.util.Map;

public class HPCToolkitProfile extends AProfile
{
	static Map<String,Integer> perfCounterTranslation = new HashMap<String,Integer>();
	static Map<String,Integer> lcpiTranslation = new HashMap<String,Integer>();

	static int metricCount = 50;	// Default arbitrary value

	double importance;
	double [] perfValues;
	double [] lcpiValues;

	public HPCToolkitProfile()
	{
		perfValues = new double [metricCount];
	}
	
	public static void setMetricCount(int metricCountIn)
	{
		metricCount = metricCountIn;
	}
	
	public static Map<String,Integer> getPerfCounterTranslation()
	{
		return perfCounterTranslation;
	}
	
	public static Map<String,Integer> getLCPITranslation()
	{
		return lcpiTranslation;
	}
	
	public static void setPerfCounterTranslation(Map<String,Integer> perfCounterTranslationIn)
	{
		perfCounterTranslation = perfCounterTranslationIn;
	}
	
	public static void setLCPITranslation(Map<String,Integer> lcpiTranslationIn)
	{
		lcpiTranslation = lcpiTranslationIn;
	}
	
	@Override
	public double getMetric(String key)
	{
		return perfValues[perfCounterTranslation.get(key)];
	}

	public double getMetric(int index)
	{
		return perfValues[index];
	}

	@Override
	public void setMetric(String key, double value)
	{
		perfValues[perfCounterTranslation.get(key)] = value;
	}

	public void setMetric(int index, double value)
	{
		perfValues[index] = value;
	}
}
