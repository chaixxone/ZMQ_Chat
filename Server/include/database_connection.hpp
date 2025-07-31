#pragma once

#include <string>
#include <memory>
#include <unordered_set>

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
	// returns fresh session_id string or an empty string if data is invalid
	std::string AuthorizeUser(const std::string& identity, const std::string& password, const std::string& deviceID) const;
	bool DoesUserExist(const std::string& identity) const;
	bool UserDeviceSession(const std::string& identity, const std::string& deviceID) const;
	bool DoesSessionExist(const std::string& identity, const std::string& deviceID, const std::string& sessionId) const;
	// returns message id 
	size_t StoreMessage(int chatId, const std::string& messageContent);
	// returns deleted session count - useful for logging
	int DeleteSession(const std::string& identity, const std::string& deviceID, const std::string& sessionID) const;
	std::unordered_set<std::string> GetChatClients(int chatId);

	int CreateChat();
	void AddClientToChat(const std::string& identity, int chatId) const;

private:
	sql::Driver* _driver;
	std::unique_ptr<sql::Connection> _connection;

	bool IsPasswordValid(const std::string& identity, const std::string& password) const;
	std::string HashPassword(const std::string& password) const;
	std::string GetPasswordHash(const std::string& identity) const;
	std::string CreateSession(const std::string& identity, const std::string& deviceID) const;
};

auto CreateDatabaseConnection(std::string host, std::string user, std::string password, std::string schema) -> std::unique_ptr<DatabaseConnection>;
