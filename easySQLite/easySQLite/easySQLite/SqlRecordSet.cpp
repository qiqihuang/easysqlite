#include "SqlRecordSet.h"

namespace sql
{

RecordSet::RecordSet(sqlite3* db)
	: _fields(NULL)
{
	_db = db;
	_err_msg.clear();
	_result_query = SQLITE_ERROR;
	_records.clear();
}

RecordSet::RecordSet(sqlite3* db, FieldSet* fields)
	: _fields(*fields)
{
	_db = db;
	_err_msg.clear();
	_result_query = SQLITE_ERROR;
	_records.clear();
}

RecordSet::RecordSet(sqlite3* db, Field* definition)
	: _fields(definition)
{
	_db = db;
	_err_msg.clear();
	_result_query = SQLITE_ERROR;
	_records.clear();
}

RecordSet::~RecordSet(void)
{
	close();
}

string RecordSet::errMsg()
{
	return _err_msg;
}

void RecordSet::close()
{
	_err_msg.clear();
	_records.clear();
	_result_query = SQLITE_ERROR;
}

FieldSet* RecordSet::fields()
{
	return &_fields;
}

bool RecordSet::isResult()
{
	return (_result_query == SQLITE_OK);
}

size_t RecordSet::count()
{
	return _records.size();
}

int RecordSet::on_next_record(void* param, int column_count, char** values, char** columns)
{
	RecordSet* recordset = (RecordSet*) param;

	Record record(recordset->fields());

	record.initColumnCount(column_count);

	for (int index = 0; index < column_count; index++)
	{
		char* value = values[index];

		if (Field* field = recordset->_fields.getByIndex(index))
		{
			record.initColumnValue(index, value, field->getType());
		}
	}

	recordset->_records.push_back(record);

	return DATASET_ITERATION_CONTINUE;
}

int Busy_Timeout_Callback(void *ptr, int count)
{
	printf("Timeout~~~~~~~~%d~~~~~~~~\n", count);

	sqlite3_sleep(5);

	if (15 == count)
	{
		return 0;
	}

	return 1;
}

bool RecordSet::query(string sql)
{
	close();

	char* error = 0;

	//set timeout
	sqlite3_busy_handler(_db, Busy_Timeout_Callback, (void*)_db);

#ifdef _WIN32
	_result_query = sqlite3_exec(_db, _UTF8(sql).c_str(), on_next_record, this, &error);
#else
	_result_query = sqlite3_exec(_db, sql.c_str(), on_next_record, this, &error);
#endif

	if (isResult())
	{
		return true;
	}

	if (*error)
	{
		_err_msg = error;
		printf("EXCEPTION:%s [%s]!!!!!!!!!!!!!!!!!!!\n", sql.c_str(), error);
		sqlite3_free(error);
	}

	THROW_EXCEPTION("RecordSet::query: " + errMsg())

	return false;
}

Record* RecordSet::getRecord(size_t record_index)
{
	if ((record_index >= 0) && (record_index < _records.size()))
		return &_records.at((size_t)record_index);

	return NULL;
}

string RecordSet::toString()
{
	string s;

	for (size_t record_index = 0; record_index < count(); record_index++)
	{
		if (Record* record = getRecord(record_index))
		{
			s += intToStr((integer)record_index + 1) + ". " + record->toString();
			s += "\r\n";
		}
	}

	return s;
}

Record* RecordSet::getTopRecord()
{
	if (isResult())
	{
		return getRecord(0);
	}
	return NULL;
}

Value* RecordSet::getTopRecordFirstValue()
{
	if (isResult())
	{
		if (Record* record = getRecord(0))
		{
			return record->getValue(0);
		}
	}
	return NULL;
}


//sql eof
};
