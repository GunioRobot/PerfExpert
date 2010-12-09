package edu.utexas.tacc.perfexpert.parsing;

import java.util.Map;

public interface IProfile
{
	// Which code does this profile belong to?
	public String getCodeSectionInfo();
	
	// Getters for individual or all metrics
	public double getMetric(String key);
	public Map<String, Double> getAllMetrics();
}
