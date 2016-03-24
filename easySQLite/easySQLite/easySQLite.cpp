// easySQLite.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "easySQLite/SqlCommon.h"
#include "easySQLite/SqlField.h"
#include "easySQLite/SqlDatabase.h"
#include "easySQLite/SqlTable.h"

#include "UnitTest++/src/UnitTest++.h"


using namespace sql;


void example1()
{
	//define table structure
	std::vector<Field> definition_tbPerson = 
	{
		Field("_ID", type_int, FIELD_KEY),
		Field("fname", type_text, FIELD_DEFAULT, flag_not_null),
		Field("lname", type_text, FIELD_DEFAULT, flag_not_null),
		Field("birthdate", type_time),
		//Field(DEFINITION_END),
	};

	//define database object
	sql::Database db;

	try
	{
		//open database file
		db.open("test.db");

		//define table object
		Table tbPerson(db.getHandle(), "person", definition_tbPerson);

		//remove table from database if exists
		if (tbPerson.exists())
			tbPerson.remove();

		//create new table
		tbPerson.create();

		//define new record
		Record record(tbPerson.fields());

		//set record data
		record.setString("fname", "Jan");
		record.setString("lname", "Kowalski");
		record.setTime("birthdate", time::now());

		//add 10 records
		for (int index = 0; index < 10; index++)
			tbPerson.addRecord(&record);

		//select record to update
		std::string sql = std::string("select * from ") + tbPerson.name() + " " + "WHERE _ID = 7";

		sql::RecordSet* pRecordSet = new sql::RecordSet(tbPerson.getHandle(), tbPerson.fields());
		bool bRet = pRecordSet->query(sql);
		if (Record* record = pRecordSet->getTopRecord())
		{
			record->setString("fname", "Frank");
			record->setString("lname", "Sinatra");
			record->setNull("birthdate");

			tbPerson.updateRecord(record);
		}

		//load all records
		tbPerson.open();

		//list loaded records
		for (int index = 0; index < tbPerson.recordCount(); index++)
			if (Record* record = tbPerson.getRecord(index))
				sql::log(record->toString());

		sql::log("");
		sql::log("ALL OK");

	} catch (Exception e) {
		printf("ERROR: %s\r\n", e.msg().c_str());
	}
}

void example2()
{
	std::vector<Field> definition_tbTest =
	{
		Field("_ID", type_int, FIELD_KEY),
		Field("name", type_text, FIELD_DEFAULT, flag_not_null),
		Field("valueInt", type_int),
		Field("valueDbl", type_float),
		Field("valueTxt", type_text),
		Field("valueBol", type_bool, FIELD_DEFAULT, flag_not_null),
		Field("valueTme", type_time),
		//Field(DEFINITION_END),
	};

	Database db;

	try
	{
		db.open("test.db");

		Table source(db.getHandle(), "tbTest", definition_tbTest);

		if (source.exists())
			source.remove();

		source.create();

		for (int index = 0; index < 5; index++)
		{
			Record record(source.fields());

			record.setString("name", "test_name_1");
			record.setInteger("valueInt", 1234567890);
			record.setDouble("valueDbl", 12345.67890);
			record.setString("valueTxt", "test'value'1");
			record.setBool("valueBol", true);
			record.setTime("valueTme", time::now());

			source.addRecord(&record);
		}

		if (Table* target = Table::createFromDefinition(db.getHandle(), "_backup_" + source.name(), source.fields()->getDefinition()))
		{
			if (target->backup(source))
			{
				sql::log("");
				sql::log("ALL OK");
			} else {
				sql::log(source.errMsg());
				sql::log(target->errMsg());
			}

			delete target;
		}
	} catch (Exception e) {
		printf("ERROR: %s\r\n", e.msg().c_str());
	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	//example1();
	//example2();

	sql::log("");

	return UnitTest::RunAllTests();
}
