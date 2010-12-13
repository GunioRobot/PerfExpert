package edu.utexas.tacc.perfexpert.presentation;

import edu.utexas.tacc.perfexpert.parsing.profiles.AProfile;

public abstract class APresentation
{
	// Given the summary statistics, present in whichever form that is best
	public abstract void presentSummaryProfiles(AProfile[] profiles);
}
