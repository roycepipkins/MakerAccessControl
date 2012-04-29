#include "Common.h"
#include "ConfigRequestFactory.h"
#include "ConfigRequest.h"


ConfigRequestFactory::ConfigRequestFactory(void)
{
}


ConfigRequestFactory::~ConfigRequestFactory(void)
{
}


Poco::Net::HTTPRequestHandler * ConfigRequestFactory::createRequestHandler(const Poco::Net::HTTPServerRequest & request)
{
	return new ConfigRequest();
}
