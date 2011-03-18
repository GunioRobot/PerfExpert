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

package edu.utexas.tacc.perfexpert.parsing.hpctoolkit;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.util.List;
import java.util.Properties;

import javax.xml.parsers.SAXParserFactory;

import org.apache.log4j.Logger;
import org.xml.sax.SAXException;

import edu.utexas.tacc.perfexpert.parsing.AParser;
import edu.utexas.tacc.perfexpert.parsing.profiles.hpctoolkit.HPCToolkitProfile;

public class HPCToolkitParser extends AParser
{
	private Logger log = Logger.getLogger( HPCToolkitParser.class );
	
	RandomAccessFile file;
	Properties LCPIConfig;
	List<HPCToolkitProfile> profiles = null;
	
	private String HPCDataLocation;

	public HPCToolkitParser(String HPCDataLocation, double threshold, String sourceURI, Properties LCPIConfig)
	{
		super(threshold, sourceURI);
		this.LCPIConfig = LCPIConfig;
		this.HPCDataLocation = HPCDataLocation;
	}
	
	public List<HPCToolkitProfile> getAllProfiles()
	{
		return profiles;
	}

	public List<HPCToolkitProfile> parse()
	{
		if (profiles != null && profiles.size() != 0)
		{
			// We have already parsed the input once, don't parse again;
			return profiles;
		}
		
		log.debug("Beginning to parse \"" + sourceURI + "\"");
		
		// Figure out if we are going to parse from raw data or file or something else
		int fileIndex = sourceURI.indexOf("://");
		if (sourceURI.substring(0, fileIndex).equals("file"))
		{
			String filename = sourceURI.substring("file://".length());
			String convertedFilename = filename + ".converted";
			
			// Check if File exists
			if (!new File (filename).exists())
			{
				log.error("Input XML file does not exist: " + filename);
				return profiles;
			}

			// Convert to simpler XML format to be used with parsing
			Process p;
			StringBuilder err = new StringBuilder();
			try
			{
				log.debug("Converting input file \"" + filename + "\" to flat-profile XML: \"" + convertedFilename + "\"");
				if (!new File(HPCDataLocation + "/hpcdata.sh").exists())
				{
					log.error("Could not locate HPCToolkit conversion program at: " + HPCDataLocation + "/hpcdata.sh");
					return null;
				}

				p = new ProcessBuilder("/bin/sh", HPCDataLocation + "/hpcdata.sh", "-o", convertedFilename, filename).start();
				
				// Check if there were errors
				byte [] byteArray = new byte [1024];
				while (p.getErrorStream().read(byteArray) > 0)
					err.append(new String(byteArray));
				
				if (new String(err).contains("Exception"))
				{
					log.error("Error converting input file \"" + filename + "\":\n" + err);
					return profiles;
				}
				else
				{
					log.debug("Conversion complete");
					System.out.println("Input file: \"" + filename + "\"");
				}
			}
			catch (IOException e)
			{
				log.error("Conversion failed: " + e);
				e.printStackTrace();
				return profiles;
			}
			
			// Get the big wheels turning...
			// Parse the XML now and load profiles
			process(convertedFilename);
			
			// Sort profiles in descending order by importance, ignore (0) because it contains the root (aggregate) profile
			// Collections.sort(profiles);
			for (int i=1; i<profiles.size()-1; i++)
			{
				for (int j=i+1; j<profiles.size(); j++)
				{
					if (profiles.get(i).getImportance() < profiles.get(j).getImportance())
					{
						// Swap
						HPCToolkitProfile temp = profiles.get(i);
						profiles.set(i, profiles.get(j));
						profiles.set(j, temp);
					}
				}
			}
		}
		else
			log.error("Unsupported URI type for input file in \"" + sourceURI + "\"");

		return profiles;
	}

	void doXMLParsing(String filename)
	{
		HPCToolkitXMLParser xmlParser = new HPCToolkitXMLParser();
		xmlParser.setThreshold(threshold);
		xmlParser.setLCPIConfig(LCPIConfig);

		SAXParserFactory parserFactory = SAXParserFactory.newInstance();

		try
		{
			javax.xml.parsers.SAXParser parser = parserFactory.newSAXParser();
			parser.parse(filename, xmlParser);
		}
		catch (SAXException e)
		{
			log.error("Error parsing converted file: " + filename + "\n" + e.getMessage());
			e.printStackTrace();
		}
		catch (Exception e)
		{
			log.error("Error parsing converted file: " + filename + "\n" + e.getMessage());
			e.printStackTrace();
		}

		this.profiles = xmlParser.getProfileList(); 
		log.debug("Recorded " + profiles.size() + " profiles");
	}
	
	void process(String filename)
	{
		// Fire up the XML parser and get the list of collected profiles
		// Profiles are saved from within the method.. Bad design? Likely!
		doXMLParsing(filename);
		
		if (profiles.size() == 0)
		{
			log.error("Collected zero profiles from HPCToolkit. Is the input file valid?");
			return;
		}

		// Sanity check
		int indexOfInstructions = profiles.get(0).getConstants().getIndexOfInstructions();
		if (indexOfInstructions < 0)
		{
			log.error("Could not find PAPI_TOT_INS in the list of performance counters that HPCToolkit recorded. Is the input file valid?");
			return;
		}
		
		// Processed input file without errors, so remove the converted file from disk
		new File(filename).delete();
	}

	// Used while saving the output of the conversion
	void writeStreamToFile(InputStream is, String filename) throws IOException
	{
		File outputFile = new File(filename);
		OutputStream out = new FileOutputStream(outputFile);
		
		int len;
		byte [] buff = new byte[1024];
		while ((len = is.read(buff)) > 0)
			out.write(buff, 0, len);
		
		out.close();
		is.close();
	}
}
