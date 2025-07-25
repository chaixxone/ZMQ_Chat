#pragma once

#include <string>
#include <memory>

#ifdef CONTAINER_APP
#include <jdbc/cppconn/driver.h>
#else
#include <jdbc/mysql_driver.h>
#endif
#include <jdbc/cppconn/connection.h>

class DatabaseConnection
{
public:
	DatabaseConnection(std::string host, std::string user, std::string password, std::string schema);
	~DatabaseConnection();

	bool RegisterUser(const std::string& identity, const std::string& password) const;
	bool AuthorizeUser(const std::string& identity, const std::string& password) const;

private:
	sql::Driver* _driver;
	std::unique_ptr<sql::Connection> _connection;

	bool DoesUserExist(const std::string& identity) const;
	bool IsPasswordValid(const std::string& identity, const std::string& password) const;
	std::string HashPassword(const std::string& password) const;
	std::string GetPasswordHash(const std::string& identity) const;
};

auto CreateDatabaseConnection(std::string host, std::string user, std::string password, std::string schema) -> std::unique_ptr<DatabaseConnection>;