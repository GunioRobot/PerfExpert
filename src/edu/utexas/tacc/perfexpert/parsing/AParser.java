package edu.utexas.tacc.perfexpert.parsing;

public abstract class AParser
{
	public enum ParseMethod
	{
		STREAM,
		TREE;
	}

	// Standard file:// or data:// -based URI
	public abstract void setSourceURI(String uri);

	// Indicate which type is desired with return-type showing whether it is supported 
	public abstract boolean setParseMethod(ParseMethod m);
	
	// Methods to parse now, with the optional source URI
	public abstract void parse();
	public abstract void parse(String sourceURI);
}
