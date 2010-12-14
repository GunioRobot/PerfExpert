package edu.utexas.tacc.perfexpert.parsing;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.util.ArrayList;

import edu.utexas.tacc.perfexpert.CustomException;
import edu.utexas.tacc.perfexpert.parsing.profiles.AProfile;
import edu.utexas.tacc.perfexpert.parsing.profiles.GenericProfile;

public class HPCToolkitParser extends AParser implements ITreeParsing
{
	RandomAccessFile file;
	ArrayList<GenericProfile> profiles = new ArrayList<GenericProfile>();
	
	final String JAVA_PATH = "/usr/bin/java";
	final String LIB_DIR = "/tmp/perfexpert/lib";
	final String classPath = LIB_DIR + "/hpcdata.jar:" + LIB_DIR + "/org.apache.xerces_2.9.0.v200909240008.jar:" + LIB_DIR + "/org.eclipse.jface_3.5.1.M20090826-0800.jar";
	final String className = "edu.rice.cs.hpc.data.framework.Application";
	
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
		// Assuming TREE-form parsing, hence not switching based on selection
		
		// Figure out if we are going to parse from raw data or file or something else
		int fileIndex = sourceURI.indexOf("://");
		if (sourceURI.substring(0, fileIndex).equals("file"))
		{
			String filename = sourceURI.substring("file://".length());
			String outputFilename = filename + ".converted";

			// Convert to simpler XML format to be used with parsing
			Process p;
			try
			{
				log.debug("Converting input file \"" + filename + "\" to simpler XML: \"" + outputFilename + "\"");
				p = new ProcessBuilder(JAVA_PATH, "-cp", classPath, className, filename).start();
				writeStreamToFile(p.getInputStream(), outputFilename);
			}
			catch (IOException e)
			{
				log.error("Conversion failed: " + e);
				e.printStackTrace();
				return;
			}
			
			log.info("Input file conversion complete: " + outputFilename);
			
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

		/*
			// Validate syntax
			try
			{
				validateInputSyntax();
			}
			catch(CustomException e)
			{
				log.error("Converted file (" + outputFilename + ") does not seem to contain a valid syntax\n" + e.getMessage()
						+ "\nCommand used for converting: " + JAVA_PATH + " -cp " + classPath + " " + className + " " + filename
						+ "\nTrying to proceed...");
			}]
		*/

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
	}
	
	void process()
	{
		String line = null;
		
		while ((line = getLine()) != null)
		{
			
		}
	}
	
	String getLine()
	{
		String line = null;
		
		try
		{
			line = file.readLine();
		}
		catch (IOException e)
		{
			;
		}
		
		return line;
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
