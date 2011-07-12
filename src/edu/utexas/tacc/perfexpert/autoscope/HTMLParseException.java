package edu.utexas.tacc.perfexpert.autoscope;

import java.text.ParseException;

public class HTMLParseException extends ParseException
{
	private static final long serialVersionUID = 1L;
	static final String __PREFIX__ = "Malformed HTML source";

	HTMLParseException()
	{
		super(__PREFIX__, 0);
	}

	HTMLParseException(String message)
	{
		super(__PREFIX__ + ": " + message, 0);
	}
}
