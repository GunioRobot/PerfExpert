package edu.utexas.tacc.perfexpert.parsing.profiles;

import java.util.HashMap;
import java.util.Map;

public class GenericProfile extends AProfile
{
	Map<String,Double> metrics = new HashMap<String,Double>();

	@Override
	public double getMetric(String key)
	{
		return metrics.get(key);
	}

	@Override
	public void setMetric(String key, double value)
	{
		metrics.put(key, value);
	}

	public Map<String, Double> getAllMetrics()
	{
		return metrics;
	}

}
