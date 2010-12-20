package edu.utexas.tacc.perfexpert.configuration;

import java.util.Collections;
import java.util.Enumeration;
import java.util.LinkedHashSet;
import java.util.Properties;
import java.util.Set;

// For ordered mapping from file to memory
class LinkedProperties extends Properties
{
	private static final long serialVersionUID = 1L;
	private final LinkedHashSet<Object> keys = new LinkedHashSet<Object>();

	public Enumeration<Object> keys()
	{
		return Collections.<Object>enumeration(keys);
	}
	
	public Set<Object> keySet()
	{
		return keys;
	}

	public Object put(Object key, Object value)
	{
		keys.add(key);
		return super.put(key, value);
	}
}
