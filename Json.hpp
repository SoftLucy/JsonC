#pragma once
#include <string>
#include <map>
#include <variant>
#include <vector>
#include <optional>

#define eat(it)		++(*it)
#define text(it)	(**it)

#define checkWhitespace(c)	((c == 0x0020) || (c == 0x000A) || (c == 0x000D) || (c == 0x0009))

#define eatWhiteSpaces(it, itEnd)	for (; ((*it) != itEnd) && checkWhitespace(text(it)); eat(it)) {} if (*it == itEnd) return {};

//returns nothing if char is not correct as error
#define expectChar(it, charcter)		if (text(it) != charcter) return {};

#define JSON_VALUE	std::variant<JsonObjectT, std::vector<JsonObjectT>, std::wstring, long long, float, bool, void *>

namespace Json {

	//Wanted to do it like this did not work aaaaaaaaaaaaaaaaaaaaaaaaa fuck
	//typedef std::map<std::wstring, JsonValue> JsonObject;
	typedef struct JsonObjectT {
		std::map<std::wstring, JSON_VALUE> m;
	}JsonObject;

	typedef JSON_VALUE JsonValue;

	//needs to be know before hand for pares array
	//parse array can't be declared after value because value uses parse array
	std::optional<JsonValue> parseValue(std::wstring::iterator* it, std::wstring::iterator itEnd);
	//same
	std::optional<JsonObject> parseJsonObject(std::wstring::iterator* it, std::wstring::iterator itEnd);
	std::wstring to_String(JsonObject root);

	std::optional<std::wstring> parseString(std::wstring::iterator* it, std::wstring::iterator itEnd) {
		expectChar(it, '"');
		eat(it);

		auto strBegin = *it;

		for (; (*it != itEnd) && (text(it) != '"'); eat(it)) {
			//ignore any \[char] exp. \" 
			if (text(it) == '\\') {
				eat(it);
				//eat(it);
			}
		}
		if (*it == itEnd) return {};

		expectChar(it, '"');
		auto ret = std::wstring{ strBegin, *it };
		eat(it);
		return ret;
	}

	std::optional<bool> parseBool(std::wstring::iterator* it, std::wstring::iterator itEnd) {
		if (text(it) == 't') {
			eat(it);
			expectChar(it, 'r');
			eat(it);
			expectChar(it, 'u');
			eat(it);
			expectChar(it, 'e');
			eat(it);
			return true;
		}

		if (text(it) == 'f') {
			eat(it);
			expectChar(it, 'a');
			eat(it);
			expectChar(it, 'l');
			eat(it);
			expectChar(it, 's');
			eat(it);
			expectChar(it, 'e');
			eat(it);
			return false;
		}
		return {};
	}

	std::optional<std::variant<long long, float>> parseNumber(std::wstring::iterator* it, std::wstring::iterator itEnd) {
		auto strBeginn = *it;

		//eat everything what is a number char
		for (;
			(*it != itEnd) && (
				(text(it) == '-') ||
				(text(it) == '0') ||
				(text(it) == '1') ||
				(text(it) == '2') ||
				(text(it) == '3') ||
				(text(it) == '4') ||
				(text(it) == '5') ||
				(text(it) == '6') ||
				(text(it) == '7') ||
				(text(it) == '8') ||
				(text(it) == '9') ||
				(text(it) == '.') ||
				//(text(it) == ',') ||		//comma as fraction does not work because comma is a json symbol
				(text(it) == 'E') ||
				(text(it) == 'e') ||
				(text(it) == '+'))
			; eat(it)) {
		}

		auto numberStr = std::wstring{ strBeginn, *it };
		try
		{
			return std::stoll(numberStr);
		}
		catch (...)
		{
			try
			{
				return std::stof(numberStr);
			}
			catch (...)
			{
				return {};
			}
		}
		return { };
	}

	bool parseNull(std::wstring::iterator* it, std::wstring::iterator itEnd) {
		if (text(it) != 'n') return false;
		eat(it);
		if (text(it) != 'u') return false;
		eat(it);
		if (text(it) != 'l') return false;
		eat(it);
		if (text(it) != 'l') return false;
		eat(it);
		return true;
	}

	std::optional<JsonValue> parseJsonArray(std::wstring::iterator* it, std::wstring::iterator itEnd) {
		std::vector<JsonObject> ret;

		expectChar(it, '[');
		eat(it);

		eatWhiteSpaces(it, itEnd);

		if (text(it) == ']') {
			eat(it);
			return ret;
		}

		for (; *it != itEnd; eat(it)) {

			auto value = parseValue(it, itEnd);
			if (!value) return {};		//error parse value
			// has to be json object not value because i could not self reference json value for vector<JsonValue>
			// so m[L""] was unavoidable
			JsonObject p;
			p.m[L""] = value.value();
			ret.push_back(p);

			if (text(it) == ']') {
				eat(it);
				return ret;
			}

			expectChar(it, ',');
		}
		return {};
	}

	std::optional<JsonValue> parseValue(std::wstring::iterator* it, std::wstring::iterator itEnd) {
		std::optional<JsonValue> ret;

		eatWhiteSpaces(it, itEnd);

		//would love to use a switch case but i does not like using auto val = ... in the cases

		//string
		if (text(it) == '"') {
			auto val = parseString(it, itEnd);
			if (!val) return {};
			ret = val.value();
		}
		//object
		else if (text(it) == '{') {
			auto val = parseJsonObject(it, itEnd);
			if (!val) return {};
			ret = val.value();
		}
		//array
		else if (text(it) == '[') {
			auto val = parseJsonArray(it, itEnd);
			if (!val) return {};
			ret = val.value();
		}
		//boolean
		else if (text(it) == 't' || text(it) == 'f') {
			auto val = parseBool(it, itEnd);
			if (!val) return {};
			ret = val.value();
		}
		//null
		else if (text(it) == 'n') {
			if (!parseNull(it, itEnd)) {
				return {};
			}
			ret = nullptr;
		}
		else {

			//try parse as number if this does not work and everthing before did not than error
			auto val = parseNumber(it, itEnd);
			if (!val) return {};
			
			if (auto pval = std::get_if<long long>(&val.value()))
				ret = *pval;
			else
				if (auto pval = std::get_if<float>(&val.value()))
					ret = *pval;
				else
					return {};
		}

		eatWhiteSpaces(it, itEnd);
		return ret;
	}

	std::optional<JsonObject> parseJsonObject(std::wstring::iterator* it, std::wstring::iterator itEnd) {
		JsonObject obj;

		expectChar(it, '{');
		eat(it);

		eatWhiteSpaces(it, itEnd);

		if (text(it) == '}') {
			eat(it);
			return obj;
		}

		for (; *it != itEnd; eat(it)) {

			eatWhiteSpaces(it, itEnd);

			expectChar(it, '"');

			auto fieldName = parseString(it, itEnd);
			if (!fieldName) return {};		//error pasing string

			eatWhiteSpaces(it, itEnd);

			expectChar(it, ':');
			eat(it);

			auto value = parseValue(it, itEnd);
			if (!value) return {};		//error parse value

			obj.m[fieldName.value()] = value.value();

			if (text(it) == '}') {
				eat(it);
				return obj;
			}

			expectChar(it, ',');
		}

		return obj;
	}

	std::optional<JsonValue> tryParse(std::wstring str) {
		auto itEnd = str.end();
		for (auto it = str.begin(); it != itEnd; ++it) {
			if (*it == '{') {
				return parseJsonObject(&it, itEnd);
			}
			if (*it == '[') {
				auto arr = parseJsonArray(&it, itEnd);
				//if (!arr) return {};	//error occurred
				//JsonObject root;
				//root.m[L""] = arr.value();
				return arr;
			}
		}
		return {};
	}

	std::wstring to_string(JsonValue value) {
		std::wstring ret;
		if (auto pval = std::get_if<JsonObject>(&value))
			ret += to_string(*pval);
		if (auto pval = std::get_if<std::vector<JsonObject>>(&value)) {
			ret += L"[\n";
			for (auto ele : *pval) {
				if (ele.m.size() == 1 && ele.m.contains(L""))
					ret += to_string(ele.m[L""]);
				else
					ret += to_string(ele);
				ret += L",";
			}
			ret.erase(ret.end() - 1);
			ret += L"]\n";
		}
		if (auto pval = std::get_if<std::wstring>(&value))
			ret += L"\"" + *pval + L"\"";
		if (auto pval = std::get_if<long long>(&value))
			ret += std::to_wstring(*pval);
		if (auto pval = std::get_if<float>(&value))
			ret += std::to_wstring(*pval);
		if (auto pval = std::get_if<bool>(&value))
			ret += std::to_wstring(*pval);
		if (auto pval = std::get_if<void*>(&value))
			ret += L"null";
		return ret;
	}

	std::wstring to_String(JsonObject root) {
		std::wstring ret = L"{\n";
		for (const auto& [key, value] : root.m) {
			ret += L"\"" + key + L"\" : ";
			ret += to_string(value);
			ret += L",\n";
		}
		ret.erase(ret.end() - 2, ret.end() - 1);
		ret += L"}\n";
		return ret;
	}

	std::optional<JsonObject> try_if_JsonObject(JsonValue value) {
		if (auto pval = std::get_if<JsonObjectT>(&value))
			return *pval;
		else
			return std::nullopt;
	}

	std::optional<std::vector<JsonValue>> try_if_Vector(JsonValue value) {
		if (auto pval = std::get_if<std::vector<JsonObjectT>>(&value)) {
			std::vector<JsonValue> ret;
			for (auto& ele : *pval) {
				ret.push_back(ele.m[L""]);
			}
			return ret;
		}
		else
			return std::nullopt;
	}

	std::optional<std::wstring> try_if_wstring(JsonValue value) {
		if (auto pval = std::get_if<std::wstring>(&value))
			return *pval;
		else
			return std::nullopt;
	}

	std::optional<long long> try_if_Number(JsonValue value) {
		if (auto pval = std::get_if<long long>(&value))
			return *pval;
		else
			return std::nullopt;
	}

	std::optional<float> try_if_float(JsonValue value) {
		if (auto pval = std::get_if<float>(&value))
			return *pval;
		else
			return std::nullopt;
	}

	std::optional<bool> try_if_bool(JsonValue value) {
		if (auto pval = std::get_if<bool>(&value))
			return *pval;
		else
			return std::nullopt;
	}

	std::optional<void*> try_if_Ptr(JsonValue value) {
		if (auto pval = std::get_if<void*>(&value))
			return *pval;
		else
			return std::nullopt;
	}
}
