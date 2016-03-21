#include <sstream>
#include "SqlValue.h"


namespace sql
{

Value::Value()
{
	setValue(NULL, type_undefined);
}

Value::Value(char* value, field_type type)
{
	setValue(value, type);
}

Value::Value(const Value& value)
{
	this->_value = value._value;
	this->_isNull = value._isNull;
	this->_type = value._type;
}

Value& Value::operator=(const Value& value)
{
	if (this != &value)
	{
		this->_value = value._value;
		this->_isNull = value._isNull;
		this->_type = value._type;
	}
	return *this;
}

bool Value::equals(Value& value)
{
	if (isNull() && value.isNull())
		return true;

	switch (_type)
	{
	case type_int:
		return (asInteger() == value.asInteger());
	case type_text:
		return (asString().compare(value.asString()) == 0);
	case type_float:
		return (asDouble() == value.asDouble());
	case type_bool:
		return (asBool() == value.asBool());
	case type_time:
		return (asTime() == value.asTime());
	}

	return false;
}

void Value::setValue(char* value, field_type type)
{
	_isNull = true;
	_value.clear();
	_type = type;

	if (value)
	{
		_isNull = false;
		_value = value;
		_type = type;
	}
}

string Value::toSql(field_type type)
{
	if (isNull())
		return "null";

	if (type == type_text)
		return "'" + quoteStr(asString()) + "'";

	if (_type == type_time)
		return intToStr(asInteger());

	return asString();
}

string Value::toString()
{
	if (isNull())
		return "null";

	return asString();
}

string Value::asString()
{
	if (_type == type_time)
	{
		time t(asInteger());
		return t.asString();
	}

	return _value;
}

integer Value::asInteger()
{
	if (isNull())
		return 0;
#if (defined ANDROID) || (defined IOS_PLATFORM) || (defined _LINUX)
	std::stringstream stream;
	stream << _value;

	integer nValue;
	stream >> nValue;
	return nValue;
#elif defined(_WIN32)
	return _atoi64(_value.c_str());
#endif
}

double Value::asDouble()
{
	if (isNull())
		return 0.0;

#if (defined ANDROID) || (defined _LINUX)
	std::stringstream stream;
	stream << _value;

	double fValue;
	stream >> fValue;
	return fValue;
#elif defined(IOS_PLATFORM)
	return atof(_value.c_str());
#elif defined(_WIN32)
	return atof(_value.c_str());
#endif
}

bool Value::asBool()
{
	if (isNull())
		return false;

	return (_value.compare("1") == 0);
}

time Value::asTime()
{
	time dt(asInteger());
	return dt;
}

void Value::setNull()
{
	_isNull = true;
	_value.clear();
}

void Value::setString(string value)
{
	_isNull = false;
	_value = value;
	_type = type_text;//add by huangqi, 2015.9.24
}

//CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

void Value::setInteger(integer value)
{
	char buffer[128];
#if (defined ANDROID) || (defined IOS_PLATFORM) || (defined _LINUX)
	std::stringstream stream;
	stream << value;
	stream >> buffer;
#elif defined(_WIN32)
	_i64toa(value, buffer, 10);
#endif

	_isNull = false;
	_value = buffer;
	_type = type_int;//add by huangqi, 2015.9.24
}

void Value::setDouble(double value)
{
	char buffer[128];

	sprintf(buffer, "%0.8f", value);

	_isNull = false;
	_value = buffer;
	_type = type_float;//add by huangqi, 2015.9.24
}

#pragma warning(default : 4996)

void Value::setBool(bool value)
{
	_isNull = false;
	_value = (value ? "1" : "0");
	_type = type_bool;//add by huangqi, 2015.9.24
}

void Value::setTime(time value)
{
	time t(value);
	_isNull = false;
	setInteger(t.asInteger());
	_type = type_time;//add by huangqi, 2015.9.24
}

bool Value::isNull()
{
	return _isNull;
}


//sql eof
};
