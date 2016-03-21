//
// Copyright (C) 2010 Piotr Zagawa
//
// Released under BSD License
//

#pragma once

#include "sqlite3.h"
#include "SqlCommon.h"
#include "SqlField.h"


namespace sql
{

class Database
{
private:
	sqlite3* _db;
	string _err_msg;
	int _result_open;

public:
	Database(void);
	~Database(void);

public:
	sqlite3* getHandle();
	string errMsg();
	bool open(string filename);
	void close();
	bool isOpen();
	bool getTableFields(string tablename, std::vector<Field>& tbDef);//add by huangqi 2015.8.17

public:
	bool transactionBegin();
	bool transactionCommit();
	bool transactionRollback();

};


//sql eof
};
