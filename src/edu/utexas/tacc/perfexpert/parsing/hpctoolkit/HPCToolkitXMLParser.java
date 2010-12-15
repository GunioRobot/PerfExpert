package edu.utexas.tacc.perfexpert.parsing.hpctoolkit;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.log4j.Logger;
import org.xml.sax.Attributes;
import org.xml.sax.helpers.DefaultHandler;

import edu.utexas.tacc.perfexpert.parsing.profiles.HPCToolkitProfile;

public class HPCToolkitXMLParser extends DefaultHandler
{
	Logger log = Logger.getLogger( HPCToolkitXMLParser.class );

	boolean skip = false;
	boolean aggregateRecorded = false;
	
	int metrics = 0;
	HPCToolkitProfile profile;
	
	List<HPCToolkitProfile> profileList = new ArrayList<HPCToolkitProfile>();

	@Override
	public void startElement(String uri, String localName, String qName, Attributes attr)
	{
		if (skip)	// Everything to be ignored till we see a particular ending element
			return;
		
		// If it a metric itself, record code section and add to Profile values
		if (qName.equals("M"))
		{
			int index = Integer.parseInt(attr.getValue("n"));
			double value = Double.parseDouble(attr.getValue("v"));

			log.debug("Found an \"M\" element");
			if (profile == null)
			{
				log.debug("Starting to record in a new HPCToolkitProfile object");
				profile = new HPCToolkitProfile();
			}

			if (aggregateRecorded == false)
			{
				log.debug("This metric marks the beginning of recording the aggregate information");
				profile.setCodeSectionInfo("Aggregate");
				aggregateRecorded = true;
			}

			profile.setMetric(index, value);
			return;
		}

		// The element we came across was not an M, so if the aggregate metrics are still hanging around,
		// flush them to the profile list
		if (profile != null)
			profileList.add(profile);
		
		// Reset the object so that subsequent elements start afresh
		profile = null;

		// Procedure
		if (qName.equals("P"))
		{
			String procedureName = attr.getValue("n");
			log.debug("Found a \"P\" element for \"" + procedureName + "\"");
			log.debug("Starting to record in a new HPCToolkitProfile object");
			profile = new HPCToolkitProfile();
			profile.setCodeSectionInfo(procedureName);
			return;
		}
		
		// Don't process F (file), LM (loaded module) or S (statement) elements
		if (qName.equals("F") || qName.equals("LM") || qName.equals("S"))
		{
			aggregateRecorded = true;
			
			log.debug("Skipping element \"" + qName + "\"");
			return;
		}
		
		// If it is a C (callsite), ignore till we see an ending C element
		if (qName.equals("C"))
		{
			aggregateRecorded = true;

			log.debug("Found a \"C\" element, skipping till corresponding ending element is found");
			skip = true;
			
			return;
		}
		
		// If it is an entry from the metric table, record the name and the index
		if (qName.equals("Metric"))
		{
			String regex = "^\\d+\\.(\\w+)\\.\\[[^\\]]*\\] \\((\\w)\\)$";
			String metricName = attr.getValue("n");
			Pattern p = Pattern.compile(regex);
			Matcher m = p.matcher(metricName);
			
			if (m.find())
			{
				int index = Integer.parseInt(attr.getValue("i"));
				String revisedMetricName = m.group(1) + (m.group(2).equals("I") ? "_I" : "");

				log.debug("Recording metric [" + index + "]: " + revisedMetricName);
				HPCToolkitProfile.getPerfCounterTranslation().put(revisedMetricName, index);
				metrics++;
			}
			else
				log.debug("Encountered \"Metric\" element that did not match the regex: " + regex + "\n"
						+ "Attribute list: " + attr);
			
			return;
		}
	}
	
	@Override
	public void endElement(String uri, String localName, String qName)
	{
		// If the MetricTable element has ended, it means we have parsed all metrics.
		// Hence we can now pass the metric count to HPCToolkitProfile
		if (qName.equals("MetricTable"))
		{
			log.debug("Ending \"MetricTable\" element found, setting metric count for HPCToolkitProfile to " + metrics);
			HPCToolkitProfile.setMetricCount(metrics);
		}
		
		// If ending C element, start processing again as normal
		if (qName.equals("C"))
		{
			aggregateRecorded = true;

			log.debug("Found an ending \"C\" element, resuming parsing as normal");
			skip = false;
		}
	}
}
