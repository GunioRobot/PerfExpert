package edu.utexas.tacc.perfexpert.parsing;

public abstract class AParser
{
	String sourceURI = null;

	public enum ParseMethod
	{
		STREAM,
		TREE;
	}
	
	public AParser()
	{
		;
	}

	public AParser(String sourceURI) throws Exception
	{
		// Using TREE-parsing as default
		this(sourceURI, ParseMethod.TREE);
	}

	public AParser(String sourceURI, ParseMethod method) throws Exception
	{
		// Shortcut
		setSourceURI(sourceURI);
		if (setParseMethod(method) == false)
			throw new Exception("Selected parse method [" + method + "] is not supported by " + this.getClass().getCanonicalName());
		
		parse();
	}

	// Standard file:// or data:// -based URI
	public void setSourceURI(String sourceURI)
	{
		this.sourceURI = sourceURI;
	}

	// Type of parsing desired (return-type showing whether it is supported) 
	public abstract boolean setParseMethod(ParseMethod method);
	
	// Start parsing
	public abstract void parse() throws Exception;
}
