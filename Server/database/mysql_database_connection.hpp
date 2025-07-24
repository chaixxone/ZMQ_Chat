#pragma once

#include "database_connection.hpp"

#include <mysql_driver.h>
#include <cppconn/connection.h>

class MySQLDatabaseConnection : public DatabaseConnection
{
public:
	MySQLDatabaseConnection(std::string host, std::string user, std::string password, std::string schema);

	bool RegisterUser(const char* identity, const char* password) const override { return true; };
	bool AuthorizeUser(const char* identity, const char* password) const override { return true; };

private:
	sql::Driver* _driver;
	std::unique_ptr<sql::Connection> _connection;

	bool DoesUserExist(const char* identity) override { return true; };
	const char* HashPassword(const char* password) override { return ""; };
	const char* GetPasswordHash(const char* identity) override { return ""; };
};