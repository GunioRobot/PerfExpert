package edu.utexas.tacc.perfexpert;

public class CustomException extends Exception
{
	private static final long serialVersionUID = 1L;

	public CustomException(String msg)
	{
		super(msg);
	}
}
