package edu.utexas.tacc.perfexpert.configuration;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Properties;

import org.apache.log4j.Logger;

public abstract class AConfigManager
{
	protected String sourceURI = null;
	protected Properties props = new Properties();
	Logger log = Logger.getLogger( AConfigManager.class );
	
	AConfigManager()
	{
		;
	}
	
	AConfigManager(String sourceURI)
	{
		this.sourceURI = sourceURI;
	}
	
	// Standard file:// or data:// -based URI
	public void setSourceURI(String sourceURI)
	{
		this.sourceURI = sourceURI;
	}
	
	public Properties getProperties()
	{
		return props;
	}
	
	// To parse the configuration source now
	public boolean readConfigSource()
	{
		int fileIndex = sourceURI.indexOf("://");
		if (sourceURI.substring(0, fileIndex).equals("file"))
		{
			String filename = sourceURI.substring("file://".length());
			try
			{
				props.load(new FileInputStream(filename));
			}
			catch (FileNotFoundException e)
			{
				log.error("Machine configuration not found in: " + sourceURI);
				return false;
			}
			catch (IOException e)
			{
				log.error("Error loading machine configuration from " + sourceURI);
				e.printStackTrace();
				return false;
			}
			
			return true;
		}
		else
		{
			log.error("Unsupported URI type for configuration file in \"" + sourceURI + "\"");
			return false;
		}
	}
	
	// Since the manner in which the configuration is saved could change,
	// we version it.
	public String getVersion()
	{
		// Default to 1.0
		return props.getProperty("version", "1.0"); 
	}
}
