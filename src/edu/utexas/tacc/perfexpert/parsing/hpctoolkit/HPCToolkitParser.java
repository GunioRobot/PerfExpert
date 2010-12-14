package edu.utexas.tacc.perfexpert.parsing.hpctoolkit;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.util.ArrayList;

import javax.xml.parsers.SAXParserFactory;

import org.apache.log4j.Logger;
import org.xml.sax.SAXException;

import edu.utexas.tacc.perfexpert.parsing.AParser;
import edu.utexas.tacc.perfexpert.parsing.ITreeParsing;
import edu.utexas.tacc.perfexpert.parsing.profiles.AProfile;
import edu.utexas.tacc.perfexpert.parsing.profiles.GenericProfile;

public class HPCToolkitParser extends AParser implements ITreeParsing
{
	private static Logger log = Logger.getLogger( HPCToolkitParser.class );
	
	RandomAccessFile file;
	ArrayList<GenericProfile> profiles = new ArrayList<GenericProfile>();
	
	final String JAVA_PATH = "/usr/bin/java";
	final String LIB_DIR = "/home/klaus/temp/perfexpert/lib";
	final String classPath = LIB_DIR + "/hpcdata.jar:" + LIB_DIR + "/org.apache.xerces_2.9.0.v200909240008.jar:" + LIB_DIR + "/org.eclipse.jface_3.5.1.M20090826-0800.jar";
	final String className = "edu.rice.cs.hpc.data.framework.Application";

	String convertedFilename;
	public HPCToolkitParser(String sourceURI)
	{
		super(sourceURI);
	}
	
	public HPCToolkitParser(String sourceURI, ParseMethod method)
	{
		super(sourceURI, method);
	}
	
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
		
		log.debug("setParseMethod() not called with unsupported parsing method. Only tree-parsing supported.");
		return false;
	}

	@Override
	public void parse()
	{
		log.debug("Beginning to parse \"" + sourceURI + "\"");
		
		// Figure out if we are going to parse from raw data or file or something else
		int fileIndex = sourceURI.indexOf("://");
		if (sourceURI.substring(0, fileIndex).equals("file"))
		{
			String filename = sourceURI.substring("file://".length());
			convertedFilename = filename + ".converted";
			
			// Check if File exists
			if (!new File (filename).exists())
			{
				log.error("Input XML file does not exist: " + filename);
				return;
			}

			// Convert to simpler XML format to be used with parsing
			Process p;
			StringBuilder err = new StringBuilder();
			try
			{
				log.debug("Converting input file \"" + filename + "\" to simpler XML: \"" + convertedFilename + "\"");
				p = new ProcessBuilder(JAVA_PATH, "-cp", classPath, className, filename).start();
				writeStreamToFile(p.getInputStream(), convertedFilename);
				
				// Check if there were errors
				byte [] byteArray = new byte [1024];
				while (p.getErrorStream().read(byteArray) > 0)
					err.append(new String(byteArray));
				
				if (new String(err).contains("Exception"))
				{
					log.error("Error converting input file \"" + filename + "\":\n" + err);
					return;
				}
			}
			catch (IOException e)
			{
				log.error("Conversion failed: " + e);
				e.printStackTrace();
				return;
			}
			
			// Conversion writes messages to standard error, so printing them here
			log.info(err);
			
			// Set up stream
			try
			{
				file = new RandomAccessFile(filename, "r");
			}
			catch (FileNotFoundException e)
			{
				log.info("Could not find input file: " + filename);
				return;
			}

			// Get the big wheels turning...
			process();			
			
			// Read profiles and add to collection (profiles)
			
			// Close open stream
			try
			{
				file.close();
			} catch (IOException e)
			{
				// Ignore
				;
			}
		}
		else
			log.error("Unsupported URI type for input file in \"" + sourceURI + "\"");
	}
	
	void process()
	{
		SAXParserFactory parserFactory = SAXParserFactory.newInstance();
		
		try
		{
			javax.xml.parsers.SAXParser parser = parserFactory.newSAXParser();
			parser.parse(convertedFilename, new HPCToolkitXMLParser());
		}
		catch (SAXException e)
		{
			log.error("Error parsing converted file: " + convertedFilename + "\n" + e.getMessage());
			e.printStackTrace();
		}
		catch (Exception e)
		{
			log.error("Error parsing converted file: " + convertedFilename + "\n" + e.getMessage());
			e.printStackTrace();
		}
	}
	
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
