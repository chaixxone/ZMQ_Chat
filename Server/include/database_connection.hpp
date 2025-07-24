#pragma once

#include <memory>

#include <mysql_driver.h>
#include <cppconn/connection.h>

class DatabaseConnection
{
public:
	DatabaseConnection(std::string host, std::string user, std::string password, std::string schema);

private:
	sql::Driver* _driver;
	std::unique_ptr<sql::Connection> _connection;
};