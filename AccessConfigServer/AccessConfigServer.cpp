// AccessConfigServer.cpp : Defines the entry point for the console application.
//

#include "Common.h"
#include "ConfigRequest.h"
#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Net/HTTPServer.h>
#include "ConfigRequestFactory.h"
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/SecureServerSocket.h>



using namespace std;
using namespace Poco;
using namespace Poco::Util;
using namespace Poco::Data;
using namespace Poco::Net;


SharedPtr<SessionPool> dbSessionPool;


class ConfigRequestServer : public ServerApplication
{

	void initialize(Application& self)
	{
		loadConfiguration(); // load default configuration files, if present
		ServerApplication::initialize(self);
	}
		
	void uninitialize()
	{
		ServerApplication::uninitialize();
	}

	int main(const std::vector<std::string>& args)
	{
		initializeSSL();
		MySQL::Connector::registerConnector();
		string connectStr = config().getString("db.connectString");

		dbSessionPool = new SessionPool("MySQL", connectStr);

		

		ConfigRequest::initMimeTypes();

		HTTPRequestHandlerFactory::Ptr handlerFactory = new ConfigRequestFactory();
		HTTPServerParams::Ptr params = new HTTPServerParams();
		SecureServerSocket socket(5400);
		HTTPServer server(handlerFactory, socket, params);

		server.start();
		waitForTerminationRequest();
		server.stop();
		uninitializeSSL();
		return 0;
	}

	
};

POCO_SERVER_MAIN(ConfigRequestServer)
