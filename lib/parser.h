#pragma once

#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>


namespace omfl {

class Parser {
public:

	enum valueType {
		boolValue,
		intValue,
		floatValue,
		stringValue,
		arrayValue, 
		section, 
		arrayIndexOutOfRange
	};

	struct Value {
		Parser* parserPtr;
		void* valuePtr;
		valueType type;

		bool IsBool() const;
		bool IsInt() const;
		bool IsFloat() const;
		bool IsString() const;
		bool IsArray() const;
		bool AsBoolOrDefault(bool defaultValue) const;
		int AsIntOrDefault(int defaultValue) const;
		float AsFloatOrDefault(float defaultValue) const;
		std::string AsStringOrDefault(std::string defaultValue) const;
		bool AsBool() const;
		int AsInt() const;
		float AsFloat() const;
		std::string AsString() const;
		Value operator[](int index) const;

		Value Get(std::string key) const;
	};

	bool isValid;
	std::map<std::string, Value> keyValue;

	bool valid() const;
	Value Get(std::string key) const;

};

Parser parse(const std::string& str);
Parser parse(const std::filesystem::path& path);

}// namespace