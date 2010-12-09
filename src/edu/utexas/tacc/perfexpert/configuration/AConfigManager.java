package edu.utexas.tacc.perfexpert.configuration;

public abstract class AConfigManager
{
	// To get individual configuration parameters
	public abstract String get(String parameter);

	// Configuration source as file:// or data://, etc.
	public abstract void setSourceURI(String uri);
	
	// To parse the configuration source now
	public abstract void readConfigSource();
}
