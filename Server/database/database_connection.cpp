#include <database_connection.hpp>

DatabaseConnection::DatabaseConnection(std::string host, std::string user, std::string password, std::string schema) : 
	_driver(get_driver_instance())
{
	_connection = std::unique_ptr<sql::Connection>(_driver->connect(std::move(host), std::move(user), std::move(password)));
	_connection->setSchema(std::move(schema));
}