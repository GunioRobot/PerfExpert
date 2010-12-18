package edu.utexas.tacc.perfexpert;

import java.util.List;

import org.apache.log4j.Logger;

import edu.utexas.tacc.perfexpert.configuration.LCPIConfigManager;
import edu.utexas.tacc.perfexpert.configuration.MachineConfigManager;
import edu.utexas.tacc.perfexpert.configuration.hpctoolkit.mathparser.MathParser;
import edu.utexas.tacc.perfexpert.parsing.hpctoolkit.HPCToolkitParser;
import edu.utexas.tacc.perfexpert.parsing.profiles.hpctoolkit.HPCToolkitProfile;

public class PerfExpert
{
	public static void main(String[] args) throws Exception
	{
		Logger log = Logger.getLogger( PerfExpert.class );

		LCPIConfigManager lcpiConfig = new LCPIConfigManager("file://config/lcpi.properties");
		lcpiConfig.readConfigSource();
		
		MachineConfigManager machineConfig = new MachineConfigManager("file://config/machine.properties");
		machineConfig.readConfigSource();
		
		HPCToolkitParser parser = new HPCToolkitParser(0.1, "file:///home/ashay/temp/perfexpert/sample-inputs/03.xml", lcpiConfig.getProperties());
		List<HPCToolkitProfile> profiles = parser.parse();
		if (profiles == null || profiles.size() == 0)
			return;

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

		MathParser mathParser = new MathParser();
		for (HPCToolkitProfile profile : profiles)
		{
			if (profile.getCodeSectionInfo().equals("Aggregate") || profile.getImportance() == -1)
				continue;

			// Compute each LCPI metric
			for (String LCPI : lcpiConfig.getLCPINames())
			{
				String formula = lcpiConfig.getProperties().getProperty(LCPI);
				int index = profile.getConstants().getLCPITranslation().get(LCPI);

				double result = mathParser.parse(formula, profile, machineConfig.getProperties()); 
				profile.setLCPI(index, result);

				// log.info(profile.getCodeSectionInfo() + ": " + LCPI + " = " + result);
			}
			
			// log.info(profile.getCodeSectionInfo() + ": " + profile.getLCPI(profile.getConstants().getLCPITranslation().get("DATA_MEM")));
			log.info(profile.getCodeSectionInfo() + ": " + profile.getMetric(profile.getConstants().getIndexOfCycles()));
			// log.info(profile.getCodeSectionInfo() + ": " + profile.getImportance()*100);
		}
	}
}
