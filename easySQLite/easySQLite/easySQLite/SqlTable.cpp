#include "SqlTable.h"


namespace sql
{

Table::Table(sqlite3* db, string tableName, const std::vector<Field>& vecFields)
	: _db(db), _tableName(tableName), _recordset(db, vecFields)
{
}

Table::Table(sqlite3* db, string tableName, FieldSet* fields)
	: _db(db), _tableName(tableName), _recordset(db, fields)
{
}

string Table::name()
{
	return _tableName;
}

string Table::getDefinition()
{
	return "CREATE TABLE " + _tableName + " (" + fields()->getDefinition() + ")";
}

FieldSet* Table::fields()
{
	return _recordset.fields();
}

string Table::toString()
{
	string s;

	for (size_t index = 0; index < fields()->count(); index++)
	{
		if (Field* f = fields()->getByIndex(index))
		{
			s += intToStr((integer)index + 1) + ". " + f->getDefinition();

			if (index < (fields()->count() - 1))
				s +="\r\n";
		}
	}

	return s;
}

string Table::errMsg()
{
	return _recordset.errMsg();
}

sqlite3* Table::getHandle()
{
	return _db;
}

bool Table::create()
{
	if (exists())
		return true;

	const string sqlDefinition = getDefinition();

	if (_recordset.query(sqlDefinition))
	{
		return true;
	}

	return false;
}

bool Table::query(string queryStr)
{
	if (_recordset.query(queryStr))
	{
		return true;
	}
	return false;
}

bool Table::open()
{
	const string queryStr = "select * from " + _tableName;

	if (_recordset.query(queryStr))
	{
		return true;
	}

	return false;
}

//example of whereCondition:
//"_ID > 40"
bool Table::open(string whereCondition)
{
	const string queryStr = "select * from " + _tableName + (whereCondition.empty() ? "" : " where " + whereCondition);

	if (_recordset.query(queryStr))
	{
		return true;
	}

	return false;
}

//example of sortBy:
//"_ID desc"
bool Table::open(string whereCondition, string sortBy)
{
	const string queryStr = "select * from " + _tableName
		+ (whereCondition.empty() ? "" : " where " + whereCondition)
		+ (sortBy.empty() ? "" : " order by " + sortBy);

	if (_recordset.query(queryStr))
	{
		return true;
	}

	return false;
}

bool Table::truncate()
{
	const string queryStr = "delete from " + _tableName;

	if (_recordset.query(queryStr))
	{
		return true;
	}

	return false;
}

bool Table::remove()
{
	const string queryStr = "drop table " + _tableName;

	if (_recordset.query(queryStr))
	{
		return true;
	}

	return false;
}

bool Table::alter(Field newField)
{
	const string queryStr = "alter table " + _tableName + " add column " + newField.getName() + " " + newField.getTypeStr();

	if (_recordset.query(queryStr))
	{
		return true;
	}

	return false;
}

bool Table::deleteRecords(string whereCondition)
{	
	const string sql = "delete from " + _tableName + (whereCondition.empty() ? "" : " where " + whereCondition);

	RecordSet rs(_db, _recordset.fields());

	if (rs.query(sql))
	{
		return true;
	}

	return false;
}

size_t Table::recordCount()
{
	return _recordset.count();
}

size_t Table::totalRecordCount()
{
	const string queryStr = "select count(*) from " + _tableName;

	RecordSet rs(_db, _recordset.fields());

	if (rs.query(queryStr))
	{
		if (Value* value = rs.getTopRecordFirstValue())
		{
			return (size_t)value->asInteger();
		}
	}

	return 0;//modify by huangqi
}

bool Table::exists()
{
	const string queryStr = "select count(*) from sqlite_master where type = 'table' and name = '" + _tableName + "'";

	RecordSet rs(_db, _recordset.fields());

	if (rs.query(queryStr))
	{
		if (Value* value = rs.getTopRecordFirstValue())
		{
			return (value->asInteger() > 0);
		}
	}

	return false;
}

Record* Table::getRecord(int record_index)
{
	//Begin: Reload _recordset
	const string queryStr = "select * from " + _tableName;

	if (!_recordset.query(queryStr))
	{
		return NULL;
	}
	//End: Add by huangqi

	return _recordset.getRecord(record_index);
}

Record* Table::getTopRecord()
{
	return getRecord(0);
}


bool Table::addRecord(Record* record)
{
	if (record)
	{
		const string sql = record->toSqlInsert(name());

		RecordSet rs(_db, _recordset.fields());

		if (rs.query(sql))
		{
			return true;
		}
	}
	return false;
}

bool Table::updateRecord(Record* record)
{
	if (record)
	{
		const string sql = record->toSqlUpdate(name());

		RecordSet rs(_db, _recordset.fields());

		if (rs.query(sql))
		{
			return true;
		}
	}
	return false;
}

bool Table::updateRecord(std::map<Field*, Value*> mapColumn)
{
	std::string sCondition;

	std::map<Field*, Value*>::iterator it;
	for (it = mapColumn.begin(); it != mapColumn.end(); ++it)
	{
		Field* field = it->first;
		if (NULL == field)
		{
			continue;
		}

		if (!field->isKeyIdField())
		{
			continue;
		}

		Value* value = it->second;
		if (NULL == value)
		{
			continue;
		}

		sCondition = " where " + field->getName() + " = " + value->toSql(field->getType());
		break;
	}

	if (sCondition.empty())
	{
		return false;
	}

	std::string sUpdateSet;

	size_t i = 0;
	for (it = mapColumn.begin(); it != mapColumn.end(); ++it, ++i)
	{
		Field* field = it->first;
		if (NULL == field)
		{
			continue;
		}

		if (field->isKeyIdField())
		{
			continue;
		}

		Value* value = it->second;
		if (NULL == value)
		{
			continue;
		}

		sUpdateSet += field->getName() + "=" + value->toSql(field->getType());
		if (i < mapColumn.size() - 1)//exclude the key field
		{
			sUpdateSet += ", ";
		}
	}

	if (sUpdateSet.empty())
	{
		return false;
	}

	string sSQL = "update " + name() + " set " + sUpdateSet + sCondition;

	RecordSet rs(_db, _recordset.fields());

	return rs.query(sSQL);
}

bool Table::copyRecords(Table& source)
{
	for (size_t index = 0; index < source.recordCount(); index++)
	{
		if (Record* record = source.getRecord(index))
		{
			if (!addRecord(record))
				return false;
		}
	}
	return true;
}

Table* Table::createFromDefinition(sqlite3* db, string tableName, string fieldsDefinition)
{
	if (FieldSet* fields = FieldSet::createFromDefinition(fieldsDefinition))
	{
		Table* table = new Table(db, tableName, fields);

		delete fields;

		return table;
	}

	return NULL;
}

bool Table::backup(Table& source)
{
	bool bResult = false;

	if (exists())
		remove();

	if (create())
	{
		if (source.open())
		{
			if (copyRecords(source))
			{
				if (source.totalRecordCount() == totalRecordCount())
					bResult = true;
			}
		}
	}

	return bResult;
}


//sql eof
};
