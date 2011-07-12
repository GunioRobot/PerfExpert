package edu.utexas.tacc.perfexpert.autoscope;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.HashMap;
import java.util.Map;

import org.apache.log4j.Logger;
import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

import com.martiansoftware.jsap.JSAP;
import com.martiansoftware.jsap.JSAPException;
import com.martiansoftware.jsap.JSAPResult;
import com.martiansoftware.jsap.Switch;
import com.martiansoftware.jsap.UnflaggedOption;
import com.martiansoftware.jsap.stringparsers.StringStringParser;

public class AutoSCOPE
{
	static final String AUTOSCOPE_URL = "http://www.cs.txstate.edu/~burtscher/research/PerfExpert/AutoSCOPE/index.php";

	public static void printHelp(JSAP parser)
	{
		System.out.println ("USAGE: autoscope " + parser.getUsage() + "\n");
		System.out.println (parser.getHelp());
	}
	
	public static void main(String[] args) throws JSAPException
	{
		Logger log = Logger.getLogger( AutoSCOPE.class );
		
		JSAP parser = new JSAP();
		Switch helpSwitch = new Switch("help", 'h', "help", "Show this help screen");
		
		Switch disableFlags = new Switch("disable-flags", (char) 0, "disable-flags", "Do not show compiler flags in suggestions");
		Switch disableExamples = new Switch("disable-examples", (char) 0, "disable-examples", "Do not show code examples in suggestions");
		UnflaggedOption inputFile = new UnflaggedOption("perfexpert.out", StringStringParser.getParser(), false, "PerfExpert output file");
		
		parser.registerParameter(helpSwitch);
		parser.registerParameter(disableFlags);
		parser.registerParameter(disableExamples);
		parser.registerParameter(inputFile);
		
		JSAPResult result = parser.parse(args);

	    printBanner();
		if (!result.success() || result.getBoolean("help"))
		{
			printHelp(parser);
			System.exit(0);
		}
		
		boolean flags = !result.getBoolean("disable-flags");
		boolean examples = !result.getBoolean("disable-examples");
		String inFile = result.getString("perfexpert.out");

	    HashMap<String,String> vars = new HashMap<String,String>();
	    if (flags)		vars.put("comp", "on");
	    if (examples)	vars.put("code", "on");
	    
	    vars.put("compiler", "intel");
	    vars.put("submitted", "submitGo");

	    String HTMLText = null;
	    
	    File f = null;
	    if (inFile != null)
	    {
	    	f = new File(inFile);
	    	if (!f.exists())
	    	{
	    		log.error("Failed opening file: " + inFile);
	    		System.exit(1);
	    	}
	    }

	    try
	    {
	    	HTMLText = postData(new URL(AUTOSCOPE_URL), f, vars);
	    }
	    catch (IOException e)
	    {
	    	log.error("Failed to make HTTP request to " + AUTOSCOPE_URL + "\n" + e.getMessage());
	    	System.exit(1);
	    }

	    try
	    {
	    	parseDocument(HTMLText);
	    }
	    catch (HTMLParseException e)
	    {
			log.error("Failed to parse HTML from " + AUTOSCOPE_URL + "\n" + e.getMessage());

			if (HTMLText != null)
			{
				try
				{
					File savedHTML = File.createTempFile("autoscope.", ".tmp");
					BufferedWriter out = new BufferedWriter(new FileWriter(savedHTML.getCanonicalPath()));
					out.write(HTMLText);
					out.close();
	
					System.out.println ("HTML saved in " + savedHTML.getCanonicalPath());
				}
				catch (IOException e2) { }
			}
	    }
	}
	
	public static void printBanner()
	{
		System.out.println ("AutoSCOPE v1.0");
		System.out.println ("Copyright (C) 2011 Texas State University");
		System.out.println ("http://www.cs.txstate.edu/~burtscher/research/PerfExpert/AutoSCOPE/\n");
	}

	public static void parseCodeSectionTable(Element codeSection) throws HTMLParseException
	{
		Elements rows = codeSection.children().select("tr");
		if (rows.size() != 1 || rows.get(0).children().size() != 2)
			throw new HTMLParseException("Code section table incorrect number of rows or columns");

		System.out.println ("\n" + rows.get(0).child(1).text() + 
			"\n===============================================================================");
	}

	public static void parseSuggestionsTable(Element suggestions) throws HTMLParseException
	{
		Elements rows = suggestions.children().select("tr");
		for (Element row : rows)
		{
			if (row.children().size() != 1 || row.child(0).children().size() > 1)
				throw new HTMLParseException("Single code suggestion row contains more than one element");

			Element rowContents = row.child(0);
			String text = rowContents.text();

			// If there are no children, this line is probably one saying: Performance of this code section is good
			if (rowContents.children().size() == 0)
				System.out.println ("* " + text);
			else
			{
				if (rowContents.html().equals("<hr style=\"width:100%\" />"))	System.out.println ("");
				else
				{
					if (rowContents.child(0).attr("style").contains("color:#000000"))	// description
						System.out.println("*" + text);
					else if (rowContents.child(0).attr("style").contains("color:#FF5B5B"))	// code example
						System.out.println("  - example:" + text);
					else if (rowContents.child(0).attr("style").contains("color:#00B050"))	// description
						System.out.println("  - compiler flag:" + text);
				}
			}
		}
	}

	public static void parseDocument(String documentText) throws HTMLParseException
	{
		if (documentText == null || documentText.length() == 0)
			throw new HTMLParseException("Document text empty");

		// Get the main table
		Document doc = Jsoup.parse(documentText);
		Elements mainTableList = doc.children().select("form > table [width=95%][style=margin:20px]");
		if (mainTableList.size() != 1)	throw new HTMLParseException("Could not obtain single main <table> element");

		Elements childTables = mainTableList.get(0).children().select("table");
		if (childTables.size() <= 1)	throw new HTMLParseException("Failed to parse code section information");

		// Ignore the header for now
		// Element headerTable = childTables.get(0);
		// parseHeaderTable(headerTable);

		for (int i=1; i<childTables.size(); i++)
		{
			Element table = childTables.get(i);

			// Every odd-numbered table contains the name of the code section
			if (i % 2 == 1)	parseCodeSectionTable(table);
			else		parseSuggestionsTable(table);
		}
	}

	public static String postData(URL url, File file, Map<String,String> vars) throws IOException
    {
	    String Boundary = "--7d021a37605f0";
        HttpURLConnection theUrlConnection = (HttpURLConnection) url.openConnection();
        theUrlConnection.setDoOutput(true);
        theUrlConnection.setDoInput(true);
        theUrlConnection.setUseCaches(false);
        //theUrlConnection.setChunkedStreamingMode(1024);

        theUrlConnection.setRequestProperty("Connection", "Keep-Alive");
        theUrlConnection.setRequestProperty("Content-Type", "multipart/form-data; boundary="
                + Boundary);

        DataOutputStream httpOut = new DataOutputStream(theUrlConnection.getOutputStream());

        String str = "--" + Boundary + "\r\n"
           	+ "Content-Disposition: form-data; name=\"dfile\"; filename=\"temp\"\r\n"
            + "Content-Type: application/octet-stream\r\n"
            + "\r\n";

        httpOut.write(str.getBytes());

        System.out.flush(); // empties buffer, before you input text
        int numBytesToRead = 1024;
        int availableBytesToRead;
        
        if (file == null)
        {
        	// Read from standard input        	
            BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
            while ((str = in.readLine()) != null)
            {
               	byte [] buffer = (str + "\n").getBytes();
               	httpOut.write(buffer);
            }
        }
        else
        {
            FileInputStream uploadFileReader = new FileInputStream(file);
            while ((availableBytesToRead = uploadFileReader.available()) > 0)
            {
            	byte[] bufferBytesRead;
            	bufferBytesRead = availableBytesToRead >= numBytesToRead ? new byte[numBytesToRead]
            			: new byte[availableBytesToRead];
                    	uploadFileReader.read(bufferBytesRead);

                httpOut.write(bufferBytesRead);
                httpOut.flush();
            }
        }

        // Write any other fields
        for (Map.Entry<String,String> entry : vars.entrySet())
        {
            str = "--" + Boundary + "\r\n"
                	+ "Content-Disposition: form-data;name=\"" + entry.getKey() + "\"\r\n"
                    + "\r\n" + entry.getValue() + "\r\n";        	

            httpOut.write(str.getBytes());
        }
        
        httpOut.write(("--" + Boundary + "--\r\n").getBytes());        
        httpOut.flush();
        httpOut.close();

        // read & parse the response
        InputStream is = theUrlConnection.getInputStream();
        final char[] buffer = new char[0x10000];

        StringBuilder out = new StringBuilder();
        InputStreamReader in = new InputStreamReader(is, "UTF-8");
        int read;
        do
        {
        	if ((read = in.read(buffer, 0, buffer.length)) > 0)
        		out.append(buffer, 0, read);
        } while (read >= 0);
        
        is.close();
        return out.toString();
    }
}
