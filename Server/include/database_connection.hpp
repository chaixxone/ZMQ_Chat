#pragma once

class __declspec(dllimport) DatabaseConnection
{
public:
	virtual ~DatabaseConnection() = default;

	virtual bool RegisterUser(const char* identity, const char* password) const = 0;
	virtual bool AuthorizeUser(const char* identity, const char* password) const = 0;

private:
	virtual bool DoesUserExist(const char* identity) = 0;
	virtual const char* HashPassword(const char* password) = 0;
	virtual const char* GetPasswordHash(const char* identity) = 0;
};

extern "C"
__declspec(dllimport)
DatabaseConnection* CreateDatabaseConnection(const char* host, const char* user, const char* password, const char* schema);