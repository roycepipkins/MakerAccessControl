#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Tuple.h>
#include <Poco/Path.h>
#include <json/json.h>
#include <Poco/Timestamp.h>

class ConfigRequest :
	public Poco::Net::HTTPRequestHandler
{
public:
	ConfigRequest();
	virtual ~ConfigRequest(void);

	virtual void handleRequest(Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse & response);
	bool isSubDir(Poco::Path& parent, Poco::Path& sub);
	void RequireAuthorization(Poco::Net::HTTPServerResponse & response);
	static std::string getMIMEType(Poco::Path& filePath);
	static std::map<std::string, std::string> mt;
	static void initMimeTypes();
	Json::Value processConfigureCommand(Json::Value& cmd);
	void GetAllUsers(Json::Value& resp);
	void SaveUser(Json::Value user, Json::Value& resp);
	void DeleteUser(Json::Value user, Json::Value& resp);
	void GetAllRoles(Json::Value& resp);

	typedef Poco::Tuple<int, std::string> UserID;
	typedef Poco::Tuple<int, std::string, Poco::Int64> RoleAssign;
	typedef Poco::Tuple<int, std::string> RoleDef;
	

};

