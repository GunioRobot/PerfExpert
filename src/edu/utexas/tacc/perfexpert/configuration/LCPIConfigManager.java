/*
    Copyright (C) 2011 The University of Texas at Austin

    This file is part of PerfExpert.

    PerfExpert is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PerfExpert is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with PerfExpert.  If not, see <http://www.gnu.org/licenses/>.

    Author: Ashay Rane
*/

package edu.utexas.tacc.perfexpert.configuration;

public class LCPIConfigManager extends AConfigManager
{
	int largestStringLength = -1;
	public LCPIConfigManager(String sourceURI)
	{
		super(sourceURI);
	}

	public final String [] getLCPINames()
	{
		// Size-1 because we do not want the "version" string
		String [] names = new String [props.size()-1];

		int ctr=0;
		for (Object key : props.keySet())
		{
			String sKey = (String) key;
			if (!sKey.equals("version"))
				names[ctr++] = (String) key;
		}

		return names;
	}
	
	public final int getLargestLCPINameLength()
	{
		if (largestStringLength != -1)
			return largestStringLength;

		int max = -1;
		int checkValue = -1;
		for (Object key : props.keySet())
		{
			String sKey = (String) key;
			checkValue = sKey.indexOf(".");
			checkValue = (checkValue == -1 ? sKey.length() : sKey.length()-checkValue);
			if (max < checkValue)
				max = checkValue;
		}

		// Adjusting so that it is at least the width of the string "performance assessment"-3
		if (max < 20)	max = 20;
		largestStringLength = max;

		return max;
	}

	// Nothing *really* special about this configuration manager
}
