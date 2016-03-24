//
// Copyright (C) 2010 Piotr Zagawa
//
// Released under BSD License
//

#pragma once

#include "SqlCommon.h"


namespace sql
{

class Field
{
public:
	friend class FieldSet;

private:
	string _name;
	field_use _use;
	field_type _type;
	int _index;
	int _flags;

public:
	Field(string name, field_type type, field_use use = FIELD_DEFAULT, int flags = flag_none);
	Field(const Field& value);

public:
	bool isKeyIdField();

public:
	int getIndex();
	string getName();
	string getTypeStr();
	field_type getType();

	bool isAutoIncrement();
	bool isNotNull();

public:
	string getDefinition();
	static Field* createFromDefinition(string value);

};


//sql eof
};
