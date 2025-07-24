#pragma once

#include <memory>

#include <sodium.h>
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

auto CreateDatabaseConnection(std::string host, std::string user, std::string password, std::string schema) 
	-> std::unique_ptr<DatabaseConnection>
{
	if (sodium_init() < 0)
	{
		std::cerr << "Sidium isn't initialized!\n";
		return nullptr;
	}

	return std::make_unique<DatabaseConnection>(std::move(host), std::move(user), std::move(password), std::move(schema));
}