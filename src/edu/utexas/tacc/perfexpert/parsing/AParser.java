package edu.utexas.tacc.perfexpert.parsing;

import org.apache.log4j.Logger;

public abstract class AParser
{
	protected String sourceURI = null;
	private static Logger log = Logger.getLogger (AParser.class);

	public enum ParseMethod
	{
		STREAM,
		TREE;
	}
	
	public AParser()
	{
		;
	}

	public AParser(String sourceURI)
	{
		// Using TREE-parsing as default
		this(sourceURI, ParseMethod.TREE);
	}

	public AParser(String sourceURI, ParseMethod method)
	{
		// Shortcut
		setSourceURI(sourceURI);
		if (setParseMethod(method) == false)
		{
			log.error("Selected parse method [" + method + "] is not supported by " + this.getClass().getCanonicalName());
			return;
		}
	}

	// Standard file:// or data:// -based URI
	public void setSourceURI(String sourceURI)
	{
		this.sourceURI = sourceURI;
	}

	// Type of parsing desired (return-type showing whether it is supported) 
	public abstract boolean setParseMethod(ParseMethod method);
	
	// Start parsing
	public abstract void parse();
}
