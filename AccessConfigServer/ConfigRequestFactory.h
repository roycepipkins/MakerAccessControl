#pragma once
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Net/StreamSocket.h>


class ConfigRequestFactory :
	public Poco::Net::HTTPRequestHandlerFactory
{
public:
	ConfigRequestFactory();
	virtual ~ConfigRequestFactory(void);

	virtual Poco::Net::HTTPRequestHandler * createRequestHandler(const Poco::Net::HTTPServerRequest & request);
};

