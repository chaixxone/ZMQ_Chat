#pragma once

#include <string>
#include <memory>

#ifdef CONTAINER_APP
#include <mysql_driver.h>
#include <cppconn/connection.h>
#else
#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/connection.h>
#endif

class DatabaseConnection
{
public:
	DatabaseConnection(std::string host, std::string user, std::string password, std::string schema);
	~DatabaseConnection();

	bool RegisterUser(const std::string& identity, const std::string& password) const;
	bool AuthorizeUser(const std::string& identity, const std::string& password) const;
	bool DoesUserExist(const std::string& identity) const;
	// returns message id 
	size_t StoreMessage(int chatId, const std::string& messageContent);

private:
	sql::Driver* _driver;
	std::unique_ptr<sql::Connection> _connection;

	bool IsPasswordValid(const std::string& identity, const std::string& password) const;
	std::string HashPassword(const std::string& password) const;
	std::string GetPasswordHash(const std::string& identity) const;
};

auto CreateDatabaseConnection(std::string host, std::string user, std::string password, std::string schema) -> std::unique_ptr<DatabaseConnection>;
