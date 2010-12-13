package edu.utexas.tacc.perfexpert.parsing;

import java.util.ArrayList;

import edu.utexas.tacc.perfexpert.parsing.profiles.AProfile;
import edu.utexas.tacc.perfexpert.parsing.profiles.GenericProfile;

public class HPCToolkitParser extends AParser implements ITreeParsing
{
	ArrayList<GenericProfile> profiles = new ArrayList<GenericProfile>();
	
	@Override
	public AProfile[] getAllProfiles()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public boolean setParseMethod(ParseMethod method)
	{
		// Currently only Tree-parsing enabled
		if (method == ParseMethod.TREE)
			return true;
		
		return false;
	}

	@Override
	public void parse() throws Exception
	{
		// Assuming TREE-form parsing, hence not switching based on selection
		
		// Figure out if we are going to parse from raw data or file or something else
		if (sourceURI.substring(0, sourceURI.indexOf("://")-1).equals("file"))
		{
			// Read file
			
			
			// Validate syntax
			validateInputSyntax();
			
			// Read metric table
			
			// Read profiles and add to collection (profiles)
		}
	}

	private void validateInputSyntax() throws Exception
	{
		// throw new Exception("Input " + sourceURI + " is not in valid format");
	}
}
