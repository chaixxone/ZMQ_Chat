#include <database_connection.hpp>

#include <iostream>

#include <sodium.h>
#include <jdbc/cppconn/prepared_statement.h>

DatabaseConnection::DatabaseConnection(std::string host, std::string user, std::string password, std::string schema) :
	_driver(get_driver_instance())
{
	_connection = std::unique_ptr<sql::Connection>(_driver->connect(std::move(host), std::move(user), std::move(password)));
	_connection->setSchema(std::move(schema));
}

DatabaseConnection::~DatabaseConnection()
{
	_connection->close();
}

auto CreateDatabaseConnection(std::string host, std::string user, std::string password, std::string schema) -> std::unique_ptr<DatabaseConnection>
{
	if (sodium_init() < 0)
	{
		std::cerr << "Failed to initialize sodium\n";
		return nullptr;
	}

	return std::make_unique<DatabaseConnection>(std::move(host), std::move(user), std::move(password), std::move(schema));
}