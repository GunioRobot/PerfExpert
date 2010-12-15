package edu.utexas.tacc.perfexpert.parsing.profiles;

public abstract class AProfile
{
	protected String codeSectionInfo = null;
	
	// Which code does this profile belong to?
	public String getCodeSectionInfo()
	{
		return this.codeSectionInfo;
	}
	
	public void setCodeSectionInfo(String codeSectionInfo)
	{
		this.codeSectionInfo = codeSectionInfo;
	}
	
	// Getters (and setters) for individual or all metrics
	public abstract double getMetric(String key);
	public abstract void setMetric(String key, double value);
}
