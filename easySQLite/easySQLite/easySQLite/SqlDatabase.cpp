#include "SqlDatabase.h"
#include "SqlRecordSet.h"
#include <time.h>

namespace sql
{

Database::Database(void)
{
	_db = NULL;
	_result_open = SQLITE_ERROR;

	close();

#if (defined ANDROID) || (defined IOS_PLATFORM) || (defined _LINUX)
	tzset();
#elif _WIN32
	//_tzset();
#endif
}

Database::~Database(void)
{
	close();
}

sqlite3* Database::getHandle()
{
	return _db;
}

string Database::errMsg()
{
	return _err_msg;
}

void Database::close()
{
	if (_db)
	{
		sqlite3_close(_db);
		_db = NULL;
		_err_msg.clear();
		_result_open = SQLITE_ERROR;
	}
}

bool Database::isOpen()
{
	return (_result_open == SQLITE_OK);
}

bool Database::open(string filename)
{
	close();

#ifdef _WIN32
	_result_open = sqlite3_open(_UTF8(filename).c_str(), &_db);
#else
	_result_open = sqlite3_open(filename.c_str(), &_db);
#endif

	if (isOpen())
	{
		return true;
	}
	else
	{
		_err_msg = sqlite3_errmsg(_db);
		printf("EXCEPTION:%s !!!!!!!!!!!!!!!!!!!\n", _err_msg.c_str());
	}

	THROW_EXCEPTION("Database::open: " + errMsg())

	return false;
}

bool Database::getTableFields(string tablename, std::vector<Field>& tbDef)
{
	sql::Field def_tbInfo[] =
	{
		sql::Field("cid", sql::type_int),
		sql::Field("name", sql::type_text),
		sql::Field("type", sql::type_text),
		sql::Field("notnull", sql::type_int),
		sql::Field("dflt_value", sql::type_text),
		sql::Field("pk", sql::type_int),
		sql::Field(sql::DEFINITION_END),
	};

	FieldSet* pFieldSet = new FieldSet(def_tbInfo);

	RecordSet rs(_db, pFieldSet);

	delete pFieldSet;
	pFieldSet = NULL;

	const string queryStr = "pragma table_info ('" + tablename + "')";

	if (!rs.query(queryStr))
	{
		return false;
	}

	size_t nCount = rs.count();
	for (size_t i=0; i<nCount; ++i)
	{
		Record* pRecord = rs.getRecord(i);
		if (NULL == pRecord)
		{
			continue;
		}

		std::string sName = pRecord->getValue("name") == NULL ? "" : pRecord->getValue("name")->asString();
		std::string sType = pRecord->getValue("type") == NULL ? "" : pRecord->getValue("type")->asString();

		field_type eType = type_undefined;
		if ("INTEGER" == sType)
		{
			eType = type_int;
		}
		else if ("TEXT" == sType)
		{
			eType = type_text;
		}
		else if ("REAL" == sType)
		{
			eType = type_float;
		}
		else
		{
			eType = type_undefined;
		}

		integer nNotNull = pRecord->getValue("notnull") == NULL ? 0 : pRecord->getValue("notnull")->asInteger();
		integer nPKey = pRecord->getValue("pk") == NULL ? 0 : pRecord->getValue("pk")->asInteger();

		field_flags eFlags = flag_none;
		if (1 == nNotNull)
		{
			eFlags = flag_not_null;
		}

		if (1 == nPKey)
		{
			eFlags = flag_primary_key;
		}

		Field field(sName, eType, eFlags);

		tbDef.push_back(field);
	}

	return true;
}

bool Database::transactionBegin()
{
	RecordSet rs(_db);

	if (rs.query("BEGIN TRANSACTION"))
		return true;

	return false;
}

bool Database::transactionCommit()
{
	RecordSet rs(_db);

	if (rs.query("COMMIT TRANSACTION"))
		return true;

	return false;
}

bool Database::transactionRollback()
{
	RecordSet rs(_db);

	if (rs.query("ROLLBACK TRANSACTION"))
		return true;

	return false;
}


//sql eof
};
