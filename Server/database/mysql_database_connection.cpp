#include "mysql_database_connection.hpp"

#include <iostream>

#include <sodium.h>
#include <cppconn/prepared_statement.h>

MySQLDatabaseConnection::MySQLDatabaseConnection(std::string host, std::string user, std::string password, std::string schema) :
	_driver(get_driver_instance())
{
	_connection = std::unique_ptr<sql::Connection>(_driver->connect(std::move(host), std::move(user), std::move(password)));
	_connection->setSchema(std::move(schema));
}

extern "C"
__declspec(dllexport)
DatabaseConnection* CreateDatabaseConnection(const char* host, const char* user, const char* password, const char* schema)
{
	if (sodium_init() < 0)
	{
		std::cerr << "Failed to initialize sodium\n";
		return nullptr;
	}

	return new MySQLDatabaseConnection(host, user, password, schema);
}