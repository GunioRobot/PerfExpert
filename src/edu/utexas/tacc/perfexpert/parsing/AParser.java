package edu.utexas.tacc.perfexpert.parsing;

public abstract class AParser
{
	protected double threshold = 0.1;
	protected String sourceURI = null;

	public AParser()
	{
		;
	}

	public AParser(double threshold, String sourceURI)
	{
		this.threshold = threshold;
		this.sourceURI = sourceURI;
	}
}
