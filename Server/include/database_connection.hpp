#pragma once

#include <memory>

#include <mysql_driver.h>
#include <cppconn/connection.h>

class DatabaseConnection
{
public:
	DatabaseConnection(std::string host, std::string user, std::string password, std::string schema);

	[[nodiscard]] bool RegisterUser(const std::string& identity, const std::string& password) const;
	[[nodiscard]] bool AuthorizeUser(const std::string& identity, const std::string& password) const;

private:
	sql::Driver* _driver;
	std::unique_ptr<sql::Connection> _connection;

	bool DoesUserExist(const std::string& identity);
	std::string HashPassword(const std::string& password);
	std::string GetPasswordHash(const std::string& identity);
};