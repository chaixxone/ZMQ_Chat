#include <database_connection.hpp>

#include <iostream>

#include <sodium.h>

#ifdef CONTAINER_APP
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#else
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#endif

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

[[nodiscard]] bool DatabaseConnection::RegisterUser(const std::string& identity, const std::string& password) const
{
	if (!DoesUserExist(identity))
	{
		std::string hashedPassword = HashPassword(password);
		auto saveUserStatement = std::unique_ptr<sql::PreparedStatement>(
			_connection->prepareStatement(
				"INSERT INTO users (identity, password_hash) VALUES (?, ?)"
			)
		);
		saveUserStatement->setString(1, identity);
		saveUserStatement->setString(2, hashedPassword);
		saveUserStatement->execute();

		return true;
	}

	return false;
}

[[nodiscard]] bool DatabaseConnection::AuthorizeUser(const std::string& identity, const std::string& password) const
{
	return DoesUserExist(identity) && IsPasswordValid(identity, password);
}

bool DatabaseConnection::DoesUserExist(const std::string& identity) const
{
	auto userQuery = std::unique_ptr<sql::PreparedStatement>(
		_connection->prepareStatement(
			"SELECT identity FROM users WHERE identity = ?"
		)
	);
	userQuery->setString(1, identity);
	std::unique_ptr<sql::ResultSet> userResult(userQuery->executeQuery());

	return userResult->rowsCount() == 1;
}

std::string DatabaseConnection::HashPassword(const std::string& password) const
{
	char hash[crypto_pwhash_STRBYTES];
	
	if (crypto_pwhash_str(
		hash,
		password.c_str(),
		password.size(),
		crypto_pwhash_OPSLIMIT_INTERACTIVE,
		crypto_pwhash_MEMLIMIT_INTERACTIVE
	) != 0) 
	{
		std::cerr << "Error occurred during generating password hash\n";
		return "";
	}

	return hash;
}

bool DatabaseConnection::IsPasswordValid(const std::string& identity, const std::string& password) const
{
	std::string hashed = GetPasswordHash(identity);
	return crypto_pwhash_str_verify(hashed.c_str(), password.c_str(), password.length()) == 0;
}

std::string DatabaseConnection::GetPasswordHash(const std::string& userLogin) const
{
	auto query = std::unique_ptr<sql::PreparedStatement>(
		_connection->prepareStatement(
			"SELECT password_hash FROM users WHERE login=?"
		)
	);
	query->setString(1, userLogin);
	auto result = std::unique_ptr<sql::ResultSet>(query->executeQuery());
	
	if (result->next())
	{
		return result->getString("password_hash");
	}
	
	return "";
}
