package edu.utexas.tacc.perfexpert.aggregation;

import edu.utexas.tacc.perfexpert.parsing.profiles.AProfile;

public abstract class AAggregator
{
	// The idea is that the Aggregator class would churn all of the detailed section profiles to generate summary stats 
	public abstract void setSectionProfiles(AProfile[] profiles);
	
	// Summary stats
	public abstract AProfile[] getSummaryProfiles();
}
