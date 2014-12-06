///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Config.h
//
// Following file contains class retrieving config files
// 
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Created by: Vilem Otte <vilem.otte@post.cz>
//
// Log:
// - Initial file created
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

//////////////////////////////////////////////////////////////////////////////// ///////////////////
// Header section

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "../Math/Numeric/Float4.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Class & Structures definition

namespace OpenTracerCore
{
	/// <summary>
	/// The following class parses constants from our own custom format for holding constant 
	/// data.
	/// </summary>
	class Config
	{
	private:
		/// <summary>Constant types</summary>
		enum tConstant
		{
			CONSTANT_INT,
			CONSTANT_FLOAT,
			CONSTANT_STRING,
			CONSTANT_FLOAT4
		};

		/// <summary>Constant tuple, containing its type and value</summary>
		struct sConstant
		{
			tConstant type;
			void* data;

			/// <summary>Float constructor</summary>
			/// <param name="f">Value</param>
			sConstant(float f)
			{
				type = CONSTANT_FLOAT;
				data = new float(f);
			}

			/// <summary>Integer constructor</summary>
			/// <param name="i">Value</param>
			sConstant(int i)
			{
				type = CONSTANT_INT;
				data = new int(i);
			}

			/// <summary>String constructor</summary>
			/// <param name="str">Value</param>
			sConstant(const std::string& str)
			{
				type = CONSTANT_STRING;
				data = new std::string(str);
			}

			/// <summary>Float4 constructor</summary>
			/// <param name="v">Value</param>
			sConstant(const float4& v)
			{
				type = CONSTANT_FLOAT4;
				data = new float4(v);
			}

			/// <summary>Destructor, takes care of deletion of various data types.</summary>
			~sConstant()
			{
				switch (type)
				{
				case CONSTANT_FLOAT:
					delete (float*)data;
					break;

				case CONSTANT_INT:
					delete (int*)data;
					break;

				case CONSTANT_STRING:
					delete (std::string*)data;
					break;

				case CONSTANT_FLOAT4:
					delete (float4*)data;
					break;
				}
			}
		};

		std::map<std::string, sConstant*> mData;	// Data
		int mCurrentLine;							// Line counter (for parsing)
		std::vector<std::string> mLevel;			// Parser helper (pushdown for naming)

		/// <summary>Returns default variable for given type</summary>
		template<typename T>
		T GetDefault()
		{
			return (T)0;
		}

		/// <summary>GetDefault int specialization</summary>
		template<>
		int GetDefault<int>()
		{
			return std::numeric_limits<int>::min();
		}

		/// <summary>GetDefault float specialization</summary>
		template<>
		float GetDefault<float>()
		{
			return std::numeric_limits<float>::min();
		}

		/// <summary>GetDefault std::string specialization</summary>
		template<>
		std::string GetDefault<std::string>()
		{
			return std::string("Undefined");
		}

		/// <summary>GetDefault float4 specialization</summary>
		template<>
		float4 GetDefault<float4>()
		{
			return float4();
		}

		/// <summary>
		/// Parses the line in following file, determines between blank lines, block lines, 
		/// comments and constant lines.
		/// </summary>
		/// <param name="line">Line string</param>
		void ParseLine(const std::string& line);

		/// <summary>Parses following line holding constant, stores into data</summary>
		/// <param name="line">String holding line to parse, trimmed</param>
		void ParseConstantLine(const std::string& line);

		/// <summary>Does string contain float</summary>
		/// <param name="str">String</param>
		inline bool IsFloat(const std::string& str)
		{
			try
			{
				boost::lexical_cast<float>(str);
			}
			catch (boost::bad_lexical_cast&)
			{
				return false;
			}

			return true;
		}

		/// <summary>Does string contain int</summary>
		/// <param name="str">String</param>
		inline bool IsInteger(const std::string& str)
		{
			try
			{
				boost::lexical_cast<int>(str);
			}
			catch (boost::bad_lexical_cast&)
			{
				return false;
			}

			return true;
		}

		/// <summary>Does string contain setting string</summary>
		/// <param name="str">String</param>
		inline bool IsString(const std::string& str)
		{
			return (str[0] == '"' && str[str.length() - 1] == '"' ||
				str[0] == '\'' && str[str.length() - 1] == '\'');
		}

		/// <summary>Does string contain float4</summary>
		/// <param name="str">String</param>
		inline bool IsFloat4(const std::string& str)
		{
			return (str[0] == '(' && str[str.length() - 1] == ')');
		}

		/// <summary>
		/// Parses line containing either value or string, determining whether we're using float, 
		/// integer or string.
		/// </summary>
		/// <param name="line">String containing value</param>
		tConstant ParseType(const std::string& line);

	public:
		/// <summary>Creates Constants object, loads constants from file</summary>
		/// <param name="filename">File containing constants</param>
		Config(const std::string& filename);

		/// <summary>Destructor, removes all used data</summary>
		~Config();

		/// <summary>Set given constant to specified value</summary>
		/// <param name="name">Constant name</param>
		/// <param name="value">Constant new value</param>
		template<typename T>
		void Set(const std::string& name, const T& value)
		{
			mData.insert(std::pair<std::string, sConstant*>(name, new sConstant(value)));
		}

		/// <summary>Gets given constant</summary>
		/// <param name="name">Constant name</param>
		/// <return>Returns constant value in wanted type</return>
		template<typename T>
		T Get(const std::string& name)
		{
			std::map<std::string, sConstant*>::iterator it = mData.find(name);
			if (it != mData.end())
			{
				return *((T*)(it->second->data));
			}
			else
			{
				return GetDefault<T>();
			}
		}
	};
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// EOH

#endif