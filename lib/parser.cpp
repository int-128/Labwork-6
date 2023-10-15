#include "parser.h"

bool omfl::Parser::Value::IsBool() const {
	return type == boolValue;
}

bool omfl::Parser::Value::IsInt() const {
	return type == intValue;
}

bool omfl::Parser::Value::IsFloat() const {
	return type == floatValue;
}

bool omfl::Parser::Value::IsString() const {
	return type == stringValue;
}

bool omfl::Parser::Value::IsArray() const {
	return type == arrayValue;
}

bool omfl::Parser::Value::AsBoolOrDefault(bool defaultValue) const {
	if (type == boolValue)
		return *(bool*)valuePtr;
	else
		return defaultValue;
}

int omfl::Parser::Value::AsIntOrDefault(int defaultValue) const {
	if (type == intValue)
		return *(int*)valuePtr;
	else
		return defaultValue;
}

float omfl::Parser::Value::AsFloatOrDefault(float defaultValue) const {
	if (type == floatValue)
		return *(float*)valuePtr;
	else
		return defaultValue;
}

std::string omfl::Parser::Value::AsStringOrDefault(std::string defaultValue) const {
	if (type == stringValue)
		return *(std::string*)valuePtr;
	else
		return defaultValue;
}

bool omfl::Parser::Value::AsBool() const {
	return this->AsBoolOrDefault(bool());
}

int omfl::Parser::Value::AsInt() const {
	return this->AsIntOrDefault(int());
}

float omfl::Parser::Value::AsFloat() const {
	return this->AsFloatOrDefault(float());
}

std::string omfl::Parser::Value::AsString() const {
	return this->AsStringOrDefault(std::string());
}

omfl::Parser::Value omfl::Parser::Value::operator[](int index) const {
	std::vector<omfl::Parser::Value> vector = (*(std::vector<omfl::Parser::Value>*)valuePtr);
	if ((0 <= index) && (index < vector.size()))
		return (*(std::vector<omfl::Parser::Value>*)valuePtr)[index];
	else {
		Value stubVal;
		stubVal.type = arrayIndexOutOfRange;
		stubVal.parserPtr = nullptr;
		stubVal.valuePtr = nullptr;
		return stubVal;
	}
}

omfl::Parser::Value omfl::Parser::Value::Get(std::string key) const {
	return (*parserPtr).Get((*(std::string*)valuePtr) + "." + key);
}

bool omfl::Parser::valid() const {
	return isValid;
}

omfl::Parser::Value omfl::Parser::Get(std::string key) const {
	if (keyValue.find(key) != keyValue.end())
		return (*(keyValue.find(key))).second;
	else {
		Value sect;
		sect.parserPtr = (omfl::Parser*)this;
		sect.type = section;
		sect.valuePtr = new std::string;
		(*(std::string*)(sect.valuePtr)) = key;
		return sect;
	}
}

bool checkKeyName(std::string name) {
	for (char c : name) {
		if (!((('0' <= c) && (c <= '9')) || (('A' <= c) && (c <= 'Z')) || (('a' <= c) && (c <= 'z')) || (c == '-') || (c == '_')))
			return false;
	}
	return !name.empty();
}

bool checkSectionName(std::string name) {
	for (char c : name) {
		if (!((('0' <= c) && (c <= '9')) || (('A' <= c) && (c <= 'Z')) || (('a' <= c) && (c <= 'z')) || (c == '-') || (c == '_') || (c == '.')))
			return false;
	}
	if (name.empty())
		return false;
	if (name[0] == '.')
		return false;
	if (name[name.size() - 1] == '.')
		return false;
	return true;
}

omfl::Parser omfl::parse(const std::string& cstr) {
	std::string str = cstr + "\n";

	omfl::Parser parser;
	parser.isValid = false;
	std::string section;
	std::string key;
	std::string value;
	size_t i = 0;

	const short lineBeginPlace = 0;
	const short sectionPlace = 1;
	const short keyPlace = 2;
	const short valueBeginPlace = 3;
	const short valuePlace = 4;
	const short stringValuePlace = 5;
	const short arrayPlace = 6;
	const short nonValuePlace = 7;
	const short nothingExpectingPlace = 8;
	const short commaExpectingPlace = 9;
	
	short place = lineBeginPlace;
	bool inArray = false;
	std::vector<int> arrayIndexes;
	int cIndex = -1;

	while (i < str.size()) {
		if ((str[i] == ' ') && (place != stringValuePlace) && (place != valuePlace))
			++i;
		else if (place == lineBeginPlace) {
			if (str[i] == '[') {
				section.clear();
				place = sectionPlace;
				++i;
			}
			else if (str[i] == '\n')
				++i;
			else if (str[i] == '#')
				place = nonValuePlace;
			else {
				key.clear();
				place = keyPlace;
			}
		}
		else if (place == sectionPlace) {
			if (str[i] == ']') {
				place = nonValuePlace;
				++i;
				if (!checkSectionName(section))
					return parser;
			}
			else if (str[i] == '\n')
				return parser;
			else {
				section.push_back(str[i]);
				++i;
			}
		}
		else if (place == keyPlace) {
			if (str[i] == '=') {
				if (!value.empty())
					value.clear();
				place = valueBeginPlace;
				++i;
				if (!checkKeyName(key))
					return parser;
				if (parser.keyValue.find(key) != parser.keyValue.end())
					return parser;
			}
			else if (str[i] == '\n')
				return parser;
			else {
				key.push_back(str[i]);
				++i;
			}
		}
		else if (place == valueBeginPlace) {
			if (str[i] == '\n')
				return parser;
			else if (str[i] == '"') {
				place = stringValuePlace;
				++i;
			}
			else if (str[i] == '[') {
				omfl::Parser::Value aValue;
				aValue.type = omfl::Parser::arrayValue;
				aValue.parserPtr = &parser;
				aValue.valuePtr = new std::vector<omfl::Parser::Value>;
				std::string tempkey = key;
				if(!section.empty())
					tempkey = section + "." + key;
				if (!inArray) {
					parser.keyValue[tempkey] = aValue;
					inArray = true;
				}
				else {
					std::vector<omfl::Parser::Value>* vector = (std::vector<omfl::Parser::Value>*)(parser.keyValue[tempkey].valuePtr);
					for (int index : arrayIndexes) {
						vector = (std::vector<omfl::Parser::Value>*)((*vector)[index].valuePtr);
					}
					(*vector).push_back(aValue);
					arrayIndexes.push_back(cIndex);
				}
				cIndex = 0;
				place = arrayPlace;
				++i;
			}
			else
				place = valuePlace;
		}
		else if (place == valuePlace) {
			if ((str[i] == '\n') || (str[i] == '#') || (str[i] == ' ') || (str[i] == ',') || (str[i] == ']')) {
				bool digits = false;
				bool alphas = false;
				bool point = false;
				for (size_t j = 0; j < value.size(); ++j) {
					if (('0' <= value[j]) && (value[j] <= '9'))
						digits = true;
					else if ((('A' <= value[j]) && (value[j] <= 'Z')) || (('a' <= value[j]) && (value[j] <= 'z')))
						alphas = true;
					else if (value[j] == '.') {
						if (!point) {
							if (j == 0)
								return parser;
							else if ((j == 1) && ((value[0] == '+') || (value[0] == '-')))
								return parser;
							else if (j == value.size() - 1)
								return parser;
							point = true;
						}
						else
							return parser;
					}
					else if ((value[j] == '+') || (value[j] == '-')) {
						if (j != 0)
							return parser;
					}
					else
						return parser;
				}
				omfl::Parser::Value vValue;
				vValue.parserPtr = &parser;
				if (digits && alphas)
					return parser;
				else if (digits && (!point)) {
					vValue.type = omfl::Parser::intValue;
					std::string fullkey;
					if (section.empty())
						fullkey = key;
					else
						fullkey = section + "." + key;
					int* valuePtr = new int;
					*valuePtr = std::stoi(value);
					vValue.valuePtr = valuePtr;
					if (!inArray) {
						parser.keyValue[fullkey] = vValue;
						if (str[i] == '#')
							place = nonValuePlace;
						else
							place = nothingExpectingPlace;
					}
					else {
						std::vector<omfl::Parser::Value>* vector = (std::vector<omfl::Parser::Value>*)(parser.keyValue[fullkey].valuePtr);
						for (int index : arrayIndexes) {
							vector = (std::vector<omfl::Parser::Value>*)((*vector)[index].valuePtr);
						}
						(*vector).push_back(vValue);
						++cIndex;
						place = commaExpectingPlace;
					}
				}
				else if (digits) {
					vValue.type = omfl::Parser::floatValue;
					std::string fullkey;
					if (section.empty())
						fullkey = key;
					else
						fullkey = section + "." + key;
					float* valuePtr = new float;
					*valuePtr = std::stof(value);
					vValue.valuePtr = valuePtr;
					if (!inArray) {
						parser.keyValue[fullkey] = vValue;
						if (str[i] == '#')
							place = nonValuePlace;
						else
							place = nothingExpectingPlace;
					}
					else {
						std::vector<omfl::Parser::Value>* vector = (std::vector<omfl::Parser::Value>*)(parser.keyValue[fullkey].valuePtr);
						for (int index : arrayIndexes) {
							vector = (std::vector<omfl::Parser::Value>*)((*vector)[index].valuePtr);
						}
						(*vector).push_back(vValue);
						++cIndex;
						place = commaExpectingPlace;
					}
				}
				else if (alphas) {
					vValue.type = omfl::Parser::boolValue;
					std::string fullkey;
					if (section.empty())
						fullkey = key;
					else
						fullkey = section + "." + key;
					bool* valuePtr = new bool;
					if (value == "true")
						*valuePtr = true;
					else if (value == "false")
						*valuePtr = false;
					else
						return parser;
					vValue.valuePtr = valuePtr;
					if (!inArray) {
						parser.keyValue[fullkey] = vValue;
						if (str[i] == '#')
							place = nonValuePlace;
						else
							place = nothingExpectingPlace;
					}
					else {
						std::vector<omfl::Parser::Value>* vector = (std::vector<omfl::Parser::Value>*)(parser.keyValue[fullkey].valuePtr);
						for (int index : arrayIndexes) {
							vector = (std::vector<omfl::Parser::Value>*)((*vector)[index].valuePtr);
						}
						(*vector).push_back(vValue);
						++cIndex;
						place = commaExpectingPlace;
					}
				}
				else {
					return parser;
				}
			}
			else {
				value.push_back(str[i]);
				++i;
			}
		}
		else if (place == stringValuePlace) {
			if (str[i] == '"') {
				omfl::Parser::Value sValue;
				sValue.type = omfl::Parser::stringValue;
				sValue.parserPtr = &parser;
				std::string* valuePtr = new std::string;
				(*valuePtr).resize(value.size());
				for (size_t j = 0; j < value.size(); ++j)
					(*valuePtr)[j] = value[j];
				sValue.valuePtr = valuePtr;
				std::string fullkey;
				if (section.empty())
					fullkey = key;
				else
					fullkey = section + "." + key;
				if (!inArray) {
					parser.keyValue[fullkey] = sValue;
					if (str[i] == '#')
						place = nonValuePlace;
					else
						place = nothingExpectingPlace;
				}
				else {
					std::vector<omfl::Parser::Value>* vector = (std::vector<omfl::Parser::Value>*)(parser.keyValue[fullkey].valuePtr);
					for (int index : arrayIndexes) {
						vector = (std::vector<omfl::Parser::Value>*)((*vector)[index].valuePtr);
					}
					(*vector).push_back(sValue);
					++cIndex;
					place = commaExpectingPlace;
				}
				++i;
			}
			else if (str[i] == '\n')
				return parser;
			else {
				value.push_back(str[i]);
				++i;
			}
		}
		else if (place == arrayPlace) {
			if (str[i] == ']') {
				if (arrayIndexes.empty()) {
					inArray = false;
					place = nothingExpectingPlace;
				}
				else {
					cIndex = arrayIndexes.back() + 1;
					arrayIndexes.pop_back();
					place = commaExpectingPlace;
				}
				++i;
			}
			else if ((str[i] == '\n') || (str[i] == '#'))
				return parser;
			else {
				if (!value.empty())
					value.clear();
				place = valueBeginPlace;
			}
		}
		else if (place == nonValuePlace) {
			if (str[i] == '\n')
				place = lineBeginPlace;
			++i;
		}
		else if (place == nothingExpectingPlace) {
			if (str[i] == '\n')
				place = lineBeginPlace;
			else if (str[i] == '#')
				place = nonValuePlace;
			else
				return parser;
			++i;
		}
		else if (place == commaExpectingPlace) {
			if (str[i] == ',') {
				place = arrayPlace;
				++i;
			}
			else if (str[i] == ']')
				place = arrayPlace;
			else {
				return parser;
			}
		}
	}

	parser.isValid = true;
	return parser;
}

omfl::Parser omfl::parse(const std::filesystem::path& path) {
	std::ifstream file;
	file.open(path);
	std::string data;
	char c;
	while (file >> std::noskipws >> c) {
		data.push_back(c);
	}
	file.close();
	return parse(data);
}