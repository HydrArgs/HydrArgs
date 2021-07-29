#include <string>
#include <unordered_set>

#include <HydrArgs/fallback/parsing.hpp>

namespace HydrArgs::fallback::parsing{

	const std::unordered_set<std::string> trueStrings{"y", "Y", "YES", "yes", "Yes", "Yea", "YEA", "yea", "On", "ON", "on", "T", "t", "True", "true", "TRUE", "1", "+"};
	const std::unordered_set<std::string> falseStrings{"n", "N", "NO", "no", "No", "Nay", "NAY", "nay", "Off", "OFF", "OFF", "F", "f", "False", "false", "FALSE", "0", "-"};

	bool _parseBoolValue(const std::string &argStr, bool &value){
		if(trueStrings.contains(argStr)){
			value = true;
		} else {
			if(falseStrings.contains(argStr)){
				value = false;
			} else {
				return false;
			}
		}
		return true;
	}

	std::string parseBoolValue(const std::string &argStr, Arg *argPtr){
		auto specOptPtr = static_cast<SArg<ArgType::flag>*>(argPtr);
		if(_parseBoolValue(argStr, specOptPtr->value)){
			return "";
		}
		return std::string{"must be a boolean, but contains `"} + argStr + "`.";
	}

	std::string parseIntValue(const std::string &argStr, Arg *argPtr){
		auto specOptPtr = static_cast<SArg<ArgType::s4>*>(argPtr);
		specOptPtr->value = std::stoi(argStr);
		return "";
	}

	std::string parseFloatValue(const std::string &argStr, Arg *argPtr){
		auto specOptPtr = static_cast<SArg<ArgType::f4>*>(argPtr);
		specOptPtr->value = std::stof(argStr);
		return "";
	}

	std::string parseDoubleValue(const std::string &argStr, Arg *argPtr){
		auto specOptPtr = static_cast<SArg<ArgType::f8>*>(argPtr);
		specOptPtr->value = std::stod(argStr);
		return "";
	}


	template <ArgType typeValue> struct GetParserFuncHandler{};
	template<> struct GetParserFuncHandler<ArgType::flag> {constexpr static const auto parseFunc = parseBoolValue;};
	template<> struct GetParserFuncHandler<ArgType::s4> {constexpr static const auto parseFunc = parseIntValue;};
	template<> struct GetParserFuncHandler<ArgType::u4> {constexpr static const auto parseFunc = parseIntValue;};
	template<> struct GetParserFuncHandler<ArgType::f4> {constexpr static const auto parseFunc = parseFloatValue;};
	template<> struct GetParserFuncHandler<ArgType::f8> {constexpr static const auto parseFunc = parseDoubleValue;};


	std::string parseArgFromString(const std::string &argStr, Arg *argPtr){
		std::string errorMessage;

		auto switchCaseBodyLambda = [&] <ArgType t>() -> std::string {
			if constexpr (t == ArgType::string) {
				auto specOptPtr = static_cast<SArg<ArgType::string>*>(argPtr);
				specOptPtr->value = argStr;
			} else {
				errorMessage = GetParserFuncHandler<t>::parseFunc(argStr, argPtr);
				if(!errorMessage.empty()){
					return errorMessage;
				}
			}
			return "";
		};

		switch(argPtr->type){
			case ArgType::flag:
			{
				return switchCaseBodyLambda.operator()<ArgType::flag>();
			}
			break;
			case ArgType::s4:
			{
				return switchCaseBodyLambda.operator()<ArgType::s4>();
			}
			break;
			case ArgType::u4:
			{
				return switchCaseBodyLambda.operator()<ArgType::s4>();
			}
			break;
			case ArgType::f4:
			{
				return switchCaseBodyLambda.operator()<ArgType::f4>();
			}
			break;
			case ArgType::f8:
			{
				return switchCaseBodyLambda.operator()<ArgType::f8>();
			}
			break;
			case ArgType::string:
			case ArgType::path:
			{
				return switchCaseBodyLambda.operator()<ArgType::string>();
			}
			break;
			default:
				throw UnsupportedArgumentType{argPtr->type};
		}
	}
}
