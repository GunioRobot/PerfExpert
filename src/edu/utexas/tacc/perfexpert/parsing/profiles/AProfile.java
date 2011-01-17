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

package edu.utexas.tacc.perfexpert.parsing.profiles;

public abstract class AProfile
{
	protected String codeSectionInfo = null;
	
	// Which code does this profile belong to?
	public String getCodeSectionInfo()
	{
		return this.codeSectionInfo;
	}
	
	public void setCodeSectionInfo(String codeSectionInfo)
	{
		this.codeSectionInfo = codeSectionInfo;
	}
	
	// Getters (and setters) for individual or all metrics
	public abstract double getMetric(String key);
	public abstract void setMetric(String key, double value);
}
