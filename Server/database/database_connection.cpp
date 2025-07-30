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

#include <utils/helpers.hpp>

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

[[nodiscard]] std::string DatabaseConnection::AuthorizeUser(
	const std::string& identity, 
	const std::string& password, 
	const std::string& deviceID
) const
{
	if (DoesUserExist(identity) && IsPasswordValid(identity, password))
	{
		std::string sessionID = CreateSession(identity);
		return sessionID;
	}

	return "";
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

bool DatabaseConnection::UserDeviceSession(const std::string& identity, const std::string& deviceID) const
{
	auto deviceSessionQuery = std::unique_ptr<sql::PreparedStatement>(
		_connection->prepareStatement(
			"SELECT * FROM sessions WHERE device_id = ? AND user_identity = ?"
		)
	);
	deviceSessionQuery->setString(1, deviceID);
	deviceSessionQuery->setString(2, identity);
	std::unique_ptr<sql::ResultSet> deviceSessionResult{ deviceSessionQuery->executeQuery() };

	return deviceSessionResult->next();
}

bool DatabaseConnection::DoesSessionExist(const std::string& identity, const std::string& deviceID, const std::string& sessionId) const
{
	auto sessionQuery = std::unique_ptr<sql::PreparedStatement>(
		_connection->prepareStatement(
			"SELECT * FROM sessions WHERE device_id = ? AND session_id = ? AND user_identity = ?"
		)
	);
	sessionQuery->setString(1, deviceID);
	sessionQuery->setString(2, sessionId);
	sessionQuery->setString(3, identity);
	std::unique_ptr<sql::ResultSet> sessionResult{ sessionQuery->executeQuery() };

	return sessionResult->next();
}

std::string DatabaseConnection::CreateSession(const std::string& identity) const
{
	std::string hexSessionIdBuffer = Utils::GenerateString();

	auto insertSessionStatement = std::unique_ptr<sql::PreparedStatement>(
		_connection->prepareStatement(
			"INSERT INTO sessions (session_id, user_identity, created_at, expired_at) VALUES (?, ?, NOW(), NOW() + INTERVAL 30 DAY)"
		)
	);
	insertSessionStatement->setString(1, hexSessionIdBuffer);
	insertSessionStatement->setString(2, identity);
	insertSessionStatement->execute();

	return hexSessionIdBuffer;
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
			"SELECT password_hash FROM users WHERE identity = ?"
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

size_t DatabaseConnection::StoreMessage(int chatId, const std::string& messageContent)
{
	auto lastMessageQuery = std::unique_ptr<sql::PreparedStatement>(
		_connection->prepareStatement(
			"SELECT MAX(id) FROM messages WHERE chat_id = ?"
		)
	);
	lastMessageQuery->setInt(1, chatId);
	std::unique_ptr<sql::ResultSet> lastMessageResult{ lastMessageQuery->executeQuery() };

	size_t lastMessageID = lastMessageResult->next() ? lastMessageResult->getUInt64("id") + 1 : 0;

	auto insertMessageStatement = std::unique_ptr<sql::PreparedStatement>(
		_connection->prepareStatement(
			"INSERT INTO messages (id, chat_id, content) VALUES (?, ?, ?)"
		)
	);
	insertMessageStatement->setUInt64(1, lastMessageID);
	insertMessageStatement->setInt(2, chatId);
	insertMessageStatement->setString(3, messageContent);
	insertMessageStatement->execute();

	return lastMessageID;
}

std::unordered_set<std::string> DatabaseConnection::GetChatClients(int chatId)
{
	auto chatClientsQuery = std::unique_ptr<sql::PreparedStatement>(
		_connection->prepareStatement(
			"SELECT user_identity FROM user_chats WHERE chat_id = ?"
		)
	);
	chatClientsQuery->setInt(1, chatId);
	std::unique_ptr<sql::ResultSet> chatClientsResult{ chatClientsQuery->executeQuery() };

	std::unordered_set<std::string> chatClients;

	while (chatClientsResult->next())
	{
		chatClients.insert(chatClientsResult->getString("user_identity"));
	}

	return chatClients;
}