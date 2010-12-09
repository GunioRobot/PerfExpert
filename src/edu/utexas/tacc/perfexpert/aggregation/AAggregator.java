package edu.utexas.tacc.perfexpert.aggregation;

import edu.utexas.tacc.perfexpert.parsing.IProfile;

public abstract class AAggregator
{
	// The idea is that the Aggregator class would churn all of the detailed section profiles to generate summary stats 
	public abstract void setSectionProfiles(IProfile[] profiles);
	
	// Summary stats
	public abstract IProfile[] getSummaryProfiles();
}
