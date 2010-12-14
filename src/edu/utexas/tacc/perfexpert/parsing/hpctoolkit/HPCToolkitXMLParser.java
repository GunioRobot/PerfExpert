package edu.utexas.tacc.perfexpert.parsing.hpctoolkit;

import org.apache.log4j.Logger;
import org.xml.sax.Attributes;
import org.xml.sax.helpers.DefaultHandler;

public class HPCToolkitXMLParser extends DefaultHandler
{
	Logger log = Logger.getLogger( HPCToolkitXMLParser.class );
	
	@Override
	public void startElement(String uri, String localName, String qName, Attributes attributes)
	{
		if (qName.equals("Metric"))
		{
			;
		}
	}
}
