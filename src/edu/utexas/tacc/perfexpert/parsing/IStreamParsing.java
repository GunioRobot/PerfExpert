package edu.utexas.tacc.perfexpert.parsing;

import edu.utexas.tacc.perfexpert.parsing.profiles.AProfile;

public interface IStreamParsing
{
	// Streaming model
	public AProfile getNextProfile();
}
