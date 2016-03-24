//
// Copyright (C) 2010 Piotr Zagawa
//
// Released under BSD License
//

#pragma once

#include <map>
#include <vector>
#include "SqlField.h"


namespace sql
{

class FieldSet
{
private:
	std::vector<Field> _vec;
	std::map<string, Field*> _map;

private:
	void copy(const std::vector<Field>& vecFields);

public:
	FieldSet();
	FieldSet(const std::vector<Field>& vecFields);
	FieldSet(const FieldSet& source);

public:
	string toString();
	size_t count();
	Field* getByIndex(size_t index);
	Field* getByName(string name);
	Field* getKeyId();

public:
	string definitionHash();
	string getDefinition();
	static FieldSet* createFromDefinition(string value);

};


//sql eof
};
