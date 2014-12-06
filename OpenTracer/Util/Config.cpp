///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Config.cpp
//
// Following file implements methods defined in Config.h.
// 
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Created by: Vilem Otte <vilem.otte@post.cz>
//
// Log:
// - Initial file created
//
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Header section

#include "Config.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Declaration section

using namespace OpenTracerCore;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Definition section

/// <summary>Creates Constants object, loads constants from file</summary>
/// <param name="filename">File containing constants</param>
Config::Config(const std::string& filename)
{
	std::ifstream f;
	f.open(filename);

	if (f.is_open())
	{
		mCurrentLine = 0;
		while (f.good() && !f.eof())
		{
			mCurrentLine++;
			std::string line;
			std::getline(f, line);
			ParseLine(line);
		}

		f.close();
	}
}

/// <summary>Destructor, removes all used data</summary>
Config::~Config()
{
	for (std::map<std::string, sConstant*>::iterator it = mData.begin(); it != mData.end(); it++)
	{
		delete it->second;
	}

	mData.clear();
}

/// <summary>
/// Parses the line in following file, determines between blank lines, block lines, 
/// comments and constant lines.
/// </summary>
/// <param name="line">Line string</param>
void Config::ParseLine(const std::string& line)
{
	// Trim line
	std::string lineTrimmed = line;
	boost::algorithm::trim(lineTrimmed);

	// If line contains nothing, return
	if (lineTrimmed.length() < 1)
	{
		return;
	}

	// If line is comment, return
	if (lineTrimmed[0] == '/' && lineTrimmed[1] == '/')
	{
		return;
	}

	// If line contains separator, it is constant line
	if (lineTrimmed.find('=') != std::string::npos)
	{
		ParseConstantLine(lineTrimmed);
		return;
	}

	// If we're beginning new block, continue
	if (lineTrimmed.find('{') != std::string::npos)
	{
		return;
	}

	// If we're ending previous block, jump 1 level up
	if (lineTrimmed.find('}') != std::string::npos)
	{
		if (mLevel.size() == 0)
		{
			std::cout << "Error: Invalid file format for config" << std::endl;
		}
		mLevel.pop_back();
		return;
	}

	// Otherwise we have a keyword describing next block
	mLevel.push_back(lineTrimmed);
}

/// <summary>Parses following line holding constant, stores into data</summary>
/// <param name="line">String holding line to parse, trimmed</param>
void Config::ParseConstantLine(const std::string& line)
{
	// Separate line by equal sign
	int separator = line.find('=');
	std::string left = line.substr(0, separator);
	std::string right = line.substr(separator + 1);

	// Trim both sides
	boost::algorithm::trim(left);
	boost::algorithm::trim(right);

	// Create name
	std::string name;
	for (auto& level : mLevel)
	{
		name += level;
		name += ".";
	}
	name += left;

	// Detect double-definition of name
	std::map<std::string, sConstant*>::iterator it = mData.find(name);
	if (it != mData.end())
	{
		std::cout << "Error: Invalid file format for config" << std::endl;
	}

	// Create value type
	tConstant type = ParseType(right);

	// Set the values
	switch (type)
	{
	case CONSTANT_FLOAT:
		Set<float>(name, boost::lexical_cast<float>(right));
		break;

	case CONSTANT_INT:
		Set<int>(name, boost::lexical_cast<int>(right));
		break;

	case CONSTANT_STRING:
		Set<std::string>(name, right.substr(1, right.length() - 2));
		break;

	case CONSTANT_FLOAT4:
		std::string rightsub = right.substr(right.find_first_of('(') + 1);
		rightsub = rightsub.substr(0, rightsub.find_last_of(')') - 1);
		std::vector<std::string> tokens;
		boost::split(tokens, rightsub, boost::is_any_of(","));
		for (auto it = tokens.begin(); it != tokens.end(); it++)
		{
			boost::algorithm::trim(*it);
		}
		Set<float4>(name, float4(boost::lexical_cast<float>(tokens[0]),
			boost::lexical_cast<float>(tokens[1]),
			boost::lexical_cast<float>(tokens[2]),
			boost::lexical_cast<float>(tokens[3])));
		break;
	}
}

/// <summary>
/// Parses line containing either value or string, determining whether we're using float, 
/// integer or string.
/// </summary>
/// <param name="line">String containing value</param>
Config::tConstant Config::ParseType(const std::string& line)
{
	if (IsInteger(line))
	{
		return CONSTANT_INT;
	}
	else if (IsFloat(line))
	{
		return CONSTANT_FLOAT;
	}
	else if (IsString(line))
	{
		return CONSTANT_STRING;
	}
	else if (IsFloat4(line))
	{
		return CONSTANT_FLOAT4;
	}
	else
	{
		std::cout << "Error: Invalid file format for config" << std::endl;
		return CONSTANT_STRING;
	}
}