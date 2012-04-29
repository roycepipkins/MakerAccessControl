#include "Common.h"
#include "ConfigRequest.h"
#include <Poco/URI.h>
#include <Poco/Util/Application.h>
#include <Poco/File.h>
#include <json/json.h>
#include <map>
#include <string>
#include <vector>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>
#include <Poco/Base64Decoder.h>
#include <Poco/StringTokenizer.h>

using namespace std;
using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;
using namespace Poco::Data;

extern SharedPtr<SessionPool> dbSessionPool;

std::map<std::string, std::string> ConfigRequest::mt;

ConfigRequest::ConfigRequest()
{
	if (mt.empty()) initMimeTypes();
}


ConfigRequest::~ConfigRequest(void)
{
}

bool ConfigRequest::isSubDir(Path& parent, Path& sub)
{

	if (sub.depth() < parent.depth()) return false;

	int idx;
	for(idx = 0; idx < parent.depth(); idx++)
	{
		if (sub[idx] != parent[idx]) return false;
	}

	return true;
}

void ConfigRequest::RequireAuthorization(Poco::Net::HTTPServerResponse & response)
{
	response.setStatus(HTTPResponse::HTTP_UNAUTHORIZED);
	response.set("WWW-Authenticate", "Basic realm=\"AccessControl\"");
	response.send() << "<html><body>Not Authorized</body></html>";
}

void ConfigRequest::handleRequest(Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse & response)
{

	LayeredConfiguration& config = Application::instance().config();

	if (!request.hasCredentials())
	{
		RequireAuthorization(response);
		return;
	}
	else
	{
		string scheme;
		string creds;
		
		request.getCredentials(scheme, creds);
		stringstream credStream;
		credStream << creds;
		Base64Decoder b64d(credStream);
		b64d >> creds;
		StringTokenizer credToks(creds, ":");
		if (credToks.count() < 2) 
		{
			RequireAuthorization(response);
			return;
		}
		if (config.getString("login.username") != credToks[0] &&
			config.getString("login.password") != credToks[1])
		{
			RequireAuthorization(response);
			return;
		}

	}

	URI uri(request.getURI());
	if (uri.getPath().find("..") != string::npos) return;

	Path reqPath(Path::expand(uri.getPath()));
	string appdir = config.getString("application.dir");
	Path wwwPath(appdir);
	wwwPath.append("www");
	Path absReqPath = wwwPath;
	absReqPath.append(reqPath);
	
	string strreqpath = absReqPath.toString();
	string strwwwpath = wwwPath.toString();

	if (!isSubDir(wwwPath, absReqPath))
	{
		response.setStatus(HTTPResponse::HTTP_FORBIDDEN);
		response.send();
		return;
	}

	//TODO now we know we have a legit request. See if its a file. If so send it.
	//If its api entry point dispath the request to the json command parser or whatever

	if (uri.getPath() == "/configCommand")
	{
		Json::Value jsonReq;
		Json::Reader reader;
		if (reader.parse(request.stream(), jsonReq, false))
		{
			Json::FastWriter writer;
			Json::Value jsonResponse = processConfigureCommand(jsonReq);
			string strResponse = writer.write(jsonResponse);
			response.sendBuffer(strResponse.c_str(), strResponse.size());
		}
		else
		{
			response.setStatus(HTTPResponse::HTTP_NO_CONTENT);
			response.send();
		}
	}
	else if (absReqPath.isFile())
	{
		File reqFile(absReqPath.toString());

		if (reqFile.exists())
		{
			response.setStatus(HTTPResponse::HTTP_OK);
			response.sendFile(absReqPath.toString(), getMIMEType(absReqPath));
		}
		else
		{
			response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
			response.send();
		}
	}
	else if (uri.getPath() == "/")
	{
		File reqFile(absReqPath.append("index.html").toString());

		if (reqFile.exists())
		{
			response.setStatus(HTTPResponse::HTTP_OK);
			response.sendFile(absReqPath.toString(), getMIMEType(absReqPath));
		}
		else
		{
			response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
			response.send();
		}
	}
	else
	{
		response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
		response.send();
	}

}

Json::Value ConfigRequest::processConfigureCommand(Json::Value& cmd)
{
	Json::Value resp;
	resp["success"] = false;

	if (cmd.isMember("msgType"))
	{
		if (cmd["msgType"].asString() == "GetAllUsers")
		{
			GetAllUsers(resp);
		}

		if (cmd["msgType"].asString() == "GetAllRoles")
		{
			GetAllRoles(resp);
		}

		if (cmd["msgType"].asString() == "SaveUser")
		{
			SaveUser(cmd["user"], resp);
		}

		if (cmd["msgType"].asString() == "DeleteUser")
		{
			DeleteUser(cmd["user"], resp);
		}
	}

	return resp;
}

void ConfigRequest::GetAllRoles(Json::Value& resp)
{
	resp["success"] = true;
	resp["msgType"] = "AllRolesReply";
	try
	{
		

		Session sess(dbSessionPool->get());

		vector<RoleDef> roles;
		sess << "SELECT role_id, CONCAT(role, '') FROM roles", into(roles), now;

		Json::Value allRoles(Json::arrayValue);
		vector<RoleDef>::iterator it;
		for(it = roles.begin(); it != roles.end(); it++)
		{
			Json::Value role;
			role["role_id"] = it->get<0>();
			role["role_name"] = it->get<1>();
			allRoles.append(role);
		}
	
		resp["roles"] = allRoles;
	}
	catch(...)
	{
		resp["success"] = false;
	}
}

void ConfigRequest::DeleteUser(Json::Value user, Json::Value& resp)
{
	resp["success"] = true;
	resp["msgType"] = "DeleteUserReply";
	try
	{
		Session sess(dbSessionPool->get());
		int user_id = user["user_id"].asInt();
		sess << "DELETE FROM hashes WHERE user_id = ?", use(user_id), now;
		sess << "DELETE FROM user_roles WHERE user_id = ?",  use(user_id), now;
	}
	catch(...)
	{
		resp["success"] = false;
	}
}

void ConfigRequest::SaveUser(Json::Value user, Json::Value& resp)
{
	resp["success"] = true;
	resp["msgType"] = "SaveUserReply";
	try
	{
		Session sess(dbSessionPool->get());

		int user_id = user["user_id"].asInt();
		string user_name = user["user_name"].asString();

		sess << "DELETE FROM hashes WHERE user_id = ?", use(user_id), now;
		sess << "DELETE FROM user_roles WHERE user_id = ?",  use(user_id), now;

		int cnt, idx;
		cnt = user["passwords"].size();
		string password;
		Statement insert_pw = (sess << "INSERT INTO hashes VALUES(?,?,?)", use(user_id),  use(password),  use(user_name));
		for(idx = 0; idx < cnt; idx++)
		{
			password = user["passwords"][idx].asString();
			try
			{
				insert_pw.execute();
			}
			catch(Exception e)
			{
				string msg = e.message();
			}
		}

		cnt = user["roles"].size();
		int role_id;
		int expiration;
		int tzDiff = 0;
		string expirationStr;
		Poco::DateTime expireDt;
		Statement insert_ur = (sess << "INSERT INTO user_roles VALUES(?,?,FROM_UNIXTIME(?))", use(user_id), use(role_id), use(expiration));
		for(idx = 0; idx < cnt; idx++)
		{
			role_id = user["roles"][idx]["role_id"].asInt();
			expirationStr = user["roles"][idx]["expiration"].asString();
			expireDt = DateTimeParser::parse(expirationStr, tzDiff);
			expiration = (int)expireDt.timestamp().epochTime();
			insert_ur.execute();
		}
	}
	catch(...)
	{
		resp["success"] = false;
	}
}

void ConfigRequest::GetAllUsers(Json::Value& resp)
{
	try
	{
		Session sess(dbSessionPool->get());

		vector<UserID> users;
		sess << "SELECT DISTINCT user_id, username FROM hashes", into(users), now;

		Json::Value allUsers(Json::arrayValue);
		vector<UserID>::iterator it;
		vector<string>::iterator hit;
		for(it = users.begin(); it != users.end(); it++)
		{
			Json::Value user;
			user["user_id"] = it->get<0>();
			user["user_name"] = it->get<1>();
		
			Json::Value passwords(Json::arrayValue);
			vector<string> hashes;
			try
			{
				sess << "SELECT CONCAT(hash_id, '') FROM hashes WHERE user_id = ?" , use(it->get<0>()), into(hashes), now;
			}
			catch (Poco::Exception e)
			{
				string msg = e.message();
			}
			for(hit = hashes.begin(); hit != hashes.end(); hit++)
			{
				Json::Value password(Json::stringValue);
				password = *hit;
				passwords.append(password);
			}
			user["passwords"] = passwords;

			Json::Value roles(Json::arrayValue);
			vector<RoleAssign> roleAssigns;
			vector<RoleAssign>::iterator rit;
			try
			{
				sess << "SELECT roles.role_id, CONCAT(roles.role,''), UNIX_TIMESTAMP(user_roles.expiration) FROM roles, user_roles WHERE roles.role_id = user_roles.role_id AND user_roles.user_id = ?", use(it->get<0>()), into(roleAssigns), now;
			}
			catch(std::exception e)
			{
				string msg = e.what();
			}
			for(rit = roleAssigns.begin(); rit != roleAssigns.end(); rit++)
			{
				Json::Value roleAssign(Json::objectValue);
				roleAssign["role_id"] = rit->get<0>();
				roleAssign["role"] = rit->get<1>();
				time_t expiration = rit->get<2>();
				Poco::Timestamp expireTS = Poco::Timestamp::fromEpochTime(expiration);
				roleAssign["expiration"] = DateTimeFormatter::format(expireTS, "%Y-%m-%dT%H:%M:%S");
				roles.append(roleAssign);
			}
			user["roles"] = roles;

			allUsers.append(user);

		}

		resp["users"] = allUsers;
		resp["msgType"] = "AllUsersReply";
	}
	catch(Exception e)
	{
		string msg = e.message();
	}
}

std::string ConfigRequest::getMIMEType(Path& filePath)
{
	string ext = filePath.getExtension();
	string mimeType = "text/plain";
	map<string, string>::iterator it;
	it = mt.find(ext);
	if (it != mt.end())
	{
		mimeType = it->second;
	}
	return mimeType;
}

void ConfigRequest::initMimeTypes()
{
	mt["3dm"] = "x-world/x-3dmf";
	mt["3dmf"] = "x-world/x-3dmf";
	mt["a"] = "application/octet-stream";
	mt["aab"] = "application/x-authorware-bin";
	mt["aam"] = "application/x-authorware-map";
	mt["aas"] = "application/x-authorware-seg";
	mt["abc"] = "text/vnd.abc";
	mt["acgi"] = "text/html";
	mt["afl"] = "video/animaflex";
	mt["ai"] = "application/postscript";
	mt["aif"] = "audio/aiff";
	mt["aif"] = "audio/x-aiff";
	mt["aifc"] = "audio/aiff";
	mt["aifc"] = "audio/x-aiff";
	mt["aiff"] = "audio/aiff";
	mt["aiff"] = "audio/x-aiff";
	mt["aim"] = "application/x-aim";
	mt["aip"] = "text/x-audiosoft-intra";
	mt["ani"] = "application/x-navi-animation";
	mt["aos"] = "application/x-nokia-9000-communicator-add-on-software";
	mt["aps"] = "application/mime";
	mt["arc"] = "application/octet-stream";
	mt["arj"] = "application/arj";
	mt["arj"] = "application/octet-stream";
	mt["art"] = "image/x-jg";
	mt["asf"] = "video/x-ms-asf";
	mt["asm"] = "text/x-asm";
	mt["asp"] = "text/asp";
	mt["asx"] = "application/x-mplayer2";
	mt["asx"] = "video/x-ms-asf";
	mt["asx"] = "video/x-ms-asf-plugin";
	mt["au"] = "audio/basic";
	mt["au"] = "audio/x-au";
	mt["avi"] = "application/x-troff-msvideo";
	mt["avi"] = "video/avi";
	mt["avi"] = "video/msvideo";
	mt["avi"] = "video/x-msvideo";
	mt["avs"] = "video/avs-video";
	mt["bcpio"] = "application/x-bcpio";
	mt["bin"] = "application/mac-binary";
	mt["bin"] = "application/macbinary";
	mt["bin"] = "application/octet-stream";
	mt["bin"] = "application/x-binary";
	mt["bin"] = "application/x-macbinary";
	mt["bm"] = "image/bmp";
	mt["bmp"] = "image/bmp";
	mt["bmp"] = "image/x-windows-bmp";
	mt["boo"] = "application/book";
	mt["book"] = "application/book";
	mt["boz"] = "application/x-bzip2";
	mt["bsh"] = "application/x-bsh";
	mt["bz"] = "application/x-bzip";
	mt["bz2"] = "application/x-bzip2";
	mt["c"] = "text/plain";
	mt["c"] = "text/x-c";
	mt["c++"] = "text/plain";
	mt["cat"] = "application/vnd.ms-pki.seccat";
	mt["cc"] = "text/plain";
	mt["cc"] = "text/x-c";
	mt["ccad"] = "application/clariscad";
	mt["cco"] = "application/x-cocoa";
	mt["cdf"] = "application/cdf";
	mt["cdf"] = "application/x-cdf";
	mt["cdf"] = "application/x-netcdf";
	mt["cer"] = "application/pkix-cert";
	mt["cer"] = "application/x-x509-ca-cert";
	mt["cha"] = "application/x-chat";
	mt["chat"] = "application/x-chat";
	mt["class"] = "application/java";
	mt["class"] = "application/java-byte-code";
	mt["class"] = "application/x-java-class";
	mt["com"] = "application/octet-stream";
	mt["com"] = "text/plain";
	mt["conf"] = "text/plain";
	mt["cpio"] = "application/x-cpio";
	mt["cpp"] = "text/x-c";
	mt["cpt"] = "application/mac-compactpro";
	mt["cpt"] = "application/x-compactpro";
	mt["cpt"] = "application/x-cpt";
	mt["crl"] = "application/pkcs-crl";
	mt["crl"] = "application/pkix-crl";
	mt["crt"] = "application/pkix-cert";
	mt["crt"] = "application/x-x509-ca-cert";
	mt["crt"] = "application/x-x509-user-cert";
	mt["csh"] = "application/x-csh";
	mt["csh"] = "text/x-script.csh";
	mt["css"] = "application/x-pointplus";
	mt["css"] = "text/css";
	mt["cxx"] = "text/plain";
	mt["dcr"] = "application/x-director";
	mt["deepv"] = "application/x-deepv";
	mt["def"] = "text/plain";
	mt["der"] = "application/x-x509-ca-cert";
	mt["dif"] = "video/x-dv";
	mt["dir"] = "application/x-director";
	mt["dl"] = "video/dl";
	mt["dl"] = "video/x-dl";
	mt["doc"] = "application/msword";
	mt["dot"] = "application/msword";
	mt["dp"] = "application/commonground";
	mt["drw"] = "application/drafting";
	mt["dump"] = "application/octet-stream";
	mt["dv"] = "video/x-dv";
	mt["dvi"] = "application/x-dvi";
	mt["dwf"] = "drawing/x-dwf (old)";
	mt["dwf"] = "model/vnd.dwf";
	mt["dwg"] = "application/acad";
	mt["dwg"] = "image/vnd.dwg";
	mt["dwg"] = "image/x-dwg";
	mt["dxf"] = "application/dxf";
	mt["dxf"] = "image/vnd.dwg";
	mt["dxf"] = "image/x-dwg";
	mt["dxr"] = "application/x-director";
	mt["el"] = "text/x-script.elisp";
	mt["elc"] = "application/x-bytecode.elisp (compiled elisp)";
	mt["elc"] = "application/x-elc";
	mt["env"] = "application/x-envoy";
	mt["eps"] = "application/postscript";
	mt["es"] = "application/x-esrehber";
	mt["etx"] = "text/x-setext";
	mt["evy"] = "application/envoy";
	mt["evy"] = "application/x-envoy";
	mt["exe"] = "application/octet-stream";
	mt["f"] = "text/plain";
	mt["f"] = "text/x-fortran";
	mt["f77"] = "text/x-fortran";
	mt["f90"] = "text/plain";
	mt["f90"] = "text/x-fortran";
	mt["fdf"] = "application/vnd.fdf";
	mt["fif"] = "application/fractals";
	mt["fif"] = "image/fif";
	mt["fli"] = "video/fli";
	mt["fli"] = "video/x-fli";
	mt["flo"] = "image/florian";
	mt["flx"] = "text/vnd.fmi.flexstor";
	mt["fmf"] = "video/x-atomic3d-feature";
	mt["for"] = "text/plain";
	mt["for"] = "text/x-fortran";
	mt["fpx"] = "image/vnd.fpx";
	mt["fpx"] = "image/vnd.net-fpx";
	mt["frl"] = "application/freeloader";
	mt["funk"] = "audio/make";
	mt["g"] = "text/plain";
	mt["g3"] = "image/g3fax";
	mt["gif"] = "image/gif";
	mt["gl"] = "video/gl";
	mt["gl"] = "video/x-gl";
	mt["gsd"] = "audio/x-gsm";
	mt["gsm"] = "audio/x-gsm";
	mt["gsp"] = "application/x-gsp";
	mt["gss"] = "application/x-gss";
	mt["gtar"] = "application/x-gtar";
	mt["gz"] = "application/x-compressed";
	mt["gz"] = "application/x-gzip";
	mt["gzip"] = "application/x-gzip";
	mt["gzip"] = "multipart/x-gzip";
	mt["h"] = "text/plain";
	mt["h"] = "text/x-h";
	mt["hdf"] = "application/x-hdf";
	mt["help"] = "application/x-helpfile";
	mt["hgl"] = "application/vnd.hp-hpgl";
	mt["hh"] = "text/plain";
	mt["hh"] = "text/x-h";
	mt["hlb"] = "text/x-script";
	mt["hlp"] = "application/hlp";
	mt["hlp"] = "application/x-helpfile";
	mt["hlp"] = "application/x-winhelp";
	mt["hpg"] = "application/vnd.hp-hpgl";
	mt["hpgl"] = "application/vnd.hp-hpgl";
	mt["hqx"] = "application/binhex";
	mt["hqx"] = "application/binhex4";
	mt["hqx"] = "application/mac-binhex";
	mt["hqx"] = "application/mac-binhex40";
	mt["hqx"] = "application/x-binhex40";
	mt["hqx"] = "application/x-mac-binhex40";
	mt["hta"] = "application/hta";
	mt["htc"] = "text/x-component";
	mt["htm"] = "text/html";
	mt["html"] = "text/html";
	mt["htmls"] = "text/html";
	mt["htt"] = "text/webviewhtml";
	mt["htx"] = "text/html";
	mt["ice"] = "x-conference/x-cooltalk";
	mt["ico"] = "image/x-icon";
	mt["idc"] = "text/plain";
	mt["ief"] = "image/ief";
	mt["iefs"] = "image/ief";
	mt["iges"] = "application/iges";
	mt["iges"] = "model/iges";
	mt["igs"] = "application/iges";
	mt["igs"] = "model/iges";
	mt["ima"] = "application/x-ima";
	mt["imap"] = "application/x-httpd-imap";
	mt["inf"] = "application/inf";
	mt["ins"] = "application/x-internett-signup";
	mt["ip"] = "application/x-ip2";
	mt["isu"] = "video/x-isvideo";
	mt["it"] = "audio/it";
	mt["iv"] = "application/x-inventor";
	mt["ivr"] = "i-world/i-vrml";
	mt["ivy"] = "application/x-livescreen";
	mt["jam"] = "audio/x-jam";
	mt["jav"] = "text/plain";
	mt["jav"] = "text/x-java-source";
	mt["java"] = "text/plain";
	mt["java"] = "text/x-java-source";
	mt["jcm"] = "application/x-java-commerce";
	mt["jfif"] = "image/jpeg";
	mt["jfif"] = "image/pjpeg";
	mt["jfif-tbnl"] = "image/jpeg";
	mt["jpe"] = "image/jpeg";
	mt["jpe"] = "image/pjpeg";
	mt["jpeg"] = "image/jpeg";
	mt["jpeg"] = "image/pjpeg";
	mt["jpg"] = "image/jpeg";
	mt["jpg"] = "image/pjpeg";
	mt["jps"] = "image/x-jps";
	mt["js"] = "application/x-javascript";
	mt["jut"] = "image/jutvision";
	mt["kar"] = "audio/midi";
	mt["kar"] = "music/x-karaoke";
	mt["ksh"] = "application/x-ksh";
	mt["ksh"] = "text/x-script.ksh";
	mt["la"] = "audio/nspaudio";
	mt["la"] = "audio/x-nspaudio";
	mt["lam"] = "audio/x-liveaudio";
	mt["latex"] = "application/x-latex";
	mt["lha"] = "application/lha";
	mt["lha"] = "application/octet-stream";
	mt["lha"] = "application/x-lha";
	mt["lhx"] = "application/octet-stream";
	mt["list"] = "text/plain";
	mt["lma"] = "audio/nspaudio";
	mt["lma"] = "audio/x-nspaudio";
	mt["log"] = "text/plain";
	mt["lsp"] = "application/x-lisp";
	mt["lsp"] = "text/x-script.lisp";
	mt["lst"] = "text/plain";
	mt["lsx"] = "text/x-la-asf";
	mt["ltx"] = "application/x-latex";
	mt["lzh"] = "application/octet-stream";
	mt["lzh"] = "application/x-lzh";
	mt["lzx"] = "application/lzx";
	mt["lzx"] = "application/octet-stream";
	mt["lzx"] = "application/x-lzx";
	mt["m"] = "text/plain";
	mt["m"] = "text/x-m";
	mt["m1v"] = "video/mpeg";
	mt["m2a"] = "audio/mpeg";
	mt["m2v"] = "video/mpeg";
	mt["m3u"] = "audio/x-mpequrl";
	mt["man"] = "application/x-troff-man";
	mt["map"] = "application/x-navimap";
	mt["mar"] = "text/plain";
	mt["mbd"] = "application/mbedlet";
	mt["mc$"] = "application/x-magic-cap-package-1.0";
	mt["mcd"] = "application/mcad";
	mt["mcd"] = "application/x-mathcad";
	mt["mcf"] = "image/vasa";
	mt["mcf"] = "text/mcf";
	mt["mcp"] = "application/netmc";
	mt["me"] = "application/x-troff-me";
	mt["mht"] = "message/rfc822";
	mt["mhtml"] = "message/rfc822";
	mt["mid"] = "application/x-midi";
	mt["mid"] = "audio/midi";
	mt["mid"] = "audio/x-mid";
	mt["mid"] = "audio/x-midi";
	mt["mid"] = "music/crescendo";
	mt["mid"] = "x-music/x-midi";
	mt["midi"] = "application/x-midi";
	mt["midi"] = "audio/midi";
	mt["midi"] = "audio/x-mid";
	mt["midi"] = "audio/x-midi";
	mt["midi"] = "music/crescendo";
	mt["midi"] = "x-music/x-midi";
	mt["mif"] = "application/x-frame";
	mt["mif"] = "application/x-mif";
	mt["mime"] = "message/rfc822";
	mt["mime"] = "www/mime";
	mt["mjf"] = "audio/x-vnd.audioexplosion.mjuicemediafile";
	mt["mjpg"] = "video/x-motion-jpeg";
	mt["mm"] = "application/base64";
	mt["mm"] = "application/x-meme";
	mt["mme"] = "application/base64";
	mt["mod"] = "audio/mod";
	mt["mod"] = "audio/x-mod";
	mt["moov"] = "video/quicktime";
	mt["mov"] = "video/quicktime";
	mt["movie"] = "video/x-sgi-movie";
	mt["mp2"] = "audio/mpeg";
	mt["mp2"] = "audio/x-mpeg";
	mt["mp2"] = "video/mpeg";
	mt["mp2"] = "video/x-mpeg";
	mt["mp2"] = "video/x-mpeq2a";
	mt["mp3"] = "audio/mpeg3";
	mt["mp3"] = "audio/x-mpeg-3";
	mt["mp3"] = "video/mpeg";
	mt["mp3"] = "video/x-mpeg";
	mt["mpa"] = "audio/mpeg";
	mt["mpa"] = "video/mpeg";
	mt["mpc"] = "application/x-project";
	mt["mpe"] = "video/mpeg";
	mt["mpeg"] = "video/mpeg";
	mt["mpg"] = "audio/mpeg";
	mt["mpg"] = "video/mpeg";
	mt["mpga"] = "audio/mpeg";
	mt["mpp"] = "application/vnd.ms-project";
	mt["mpt"] = "application/x-project";
	mt["mpv"] = "application/x-project";
	mt["mpx"] = "application/x-project";
	mt["mrc"] = "application/marc";
	mt["ms"] = "application/x-troff-ms";
	mt["mv"] = "video/x-sgi-movie";
	mt["my"] = "audio/make";
	mt["mzz"] = "application/x-vnd.audioexplosion.mzz";
	mt["nap"] = "image/naplps";
	mt["naplps"] = "image/naplps";
	mt["nc"] = "application/x-netcdf";
	mt["ncm"] = "application/vnd.nokia.configuration-message";
	mt["nif"] = "image/x-niff";
	mt["niff"] = "image/x-niff";
	mt["nix"] = "application/x-mix-transfer";
	mt["nsc"] = "application/x-conference";
	mt["nvd"] = "application/x-navidoc";
	mt["o"] = "application/octet-stream";
	mt["oda"] = "application/oda";
	mt["omc"] = "application/x-omc";
	mt["omcd"] = "application/x-omcdatamaker";
	mt["omcr"] = "application/x-omcregerator";
	mt["p"] = "text/x-pascal";
	mt["p10"] = "application/pkcs10";
	mt["p10"] = "application/x-pkcs10";
	mt["p12"] = "application/pkcs-12";
	mt["p12"] = "application/x-pkcs12";
	mt["p7a"] = "application/x-pkcs7-signature";
	mt["p7c"] = "application/pkcs7-mime";
	mt["p7c"] = "application/x-pkcs7-mime";
	mt["p7m"] = "application/pkcs7-mime";
	mt["p7m"] = "application/x-pkcs7-mime";
	mt["p7r"] = "application/x-pkcs7-certreqresp";
	mt["p7s"] = "application/pkcs7-signature";
	mt["part"] = "application/pro_eng";
	mt["pas"] = "text/pascal";
	mt["pbm"] = "image/x-portable-bitmap";
	mt["pcl"] = "application/vnd.hp-pcl";
	mt["pcl"] = "application/x-pcl";
	mt["pct"] = "image/x-pict";
	mt["pcx"] = "image/x-pcx";
	mt["pdb"] = "chemical/x-pdb";
	mt["pdf"] = "application/pdf";
	mt["pfunk"] = "audio/make";
	mt["pfunk"] = "audio/make.my.funk";
	mt["pgm"] = "image/x-portable-graymap";
	mt["pgm"] = "image/x-portable-greymap";
	mt["pic"] = "image/pict";
	mt["pict"] = "image/pict";
	mt["pkg"] = "application/x-newton-compatible-pkg";
	mt["pko"] = "application/vnd.ms-pki.pko";
	mt["pl"] = "text/plain";
	mt["pl"] = "text/x-script.perl";
	mt["plx"] = "application/x-pixclscript";
	mt["pm"] = "image/x-xpixmap";
	mt["pm"] = "text/x-script.perl-module";
	mt["pm4"] = "application/x-pagemaker";
	mt["pm5"] = "application/x-pagemaker";
	mt["png"] = "image/png";
	mt["pnm"] = "application/x-portable-anymap";
	mt["pnm"] = "image/x-portable-anymap";
	mt["pot"] = "application/mspowerpoint";
	mt["pot"] = "application/vnd.ms-powerpoint";
	mt["pov"] = "model/x-pov";
	mt["ppa"] = "application/vnd.ms-powerpoint";
	mt["ppm"] = "image/x-portable-pixmap";
	mt["pps"] = "application/mspowerpoint";
	mt["pps"] = "application/vnd.ms-powerpoint";
	mt["ppt"] = "application/mspowerpoint";
	mt["ppt"] = "application/powerpoint";
	mt["ppt"] = "application/vnd.ms-powerpoint";
	mt["ppt"] = "application/x-mspowerpoint";
	mt["ppz"] = "application/mspowerpoint";
	mt["pre"] = "application/x-freelance";
	mt["prt"] = "application/pro_eng";
	mt["ps"] = "application/postscript";
	mt["psd"] = "application/octet-stream";
	mt["pvu"] = "paleovu/x-pv";
	mt["pwz"] = "application/vnd.ms-powerpoint";
	mt["py"] = "text/x-script.phyton";
	mt["pyc"] = "applicaiton/x-bytecode.python";
	mt["qcp"] = "audio/vnd.qcelp";
	mt["qd3"] = "x-world/x-3dmf";
	mt["qd3d"] = "x-world/x-3dmf";
	mt["qif"] = "image/x-quicktime";
	mt["qt"] = "video/quicktime";
	mt["qtc"] = "video/x-qtc";
	mt["qti"] = "image/x-quicktime";
	mt["qtif"] = "image/x-quicktime";
	mt["ra"] = "audio/x-pn-realaudio";
	mt["ra"] = "audio/x-pn-realaudio-plugin";
	mt["ra"] = "audio/x-realaudio";
	mt["ram"] = "audio/x-pn-realaudio";
	mt["ras"] = "application/x-cmu-raster";
	mt["ras"] = "image/cmu-raster";
	mt["ras"] = "image/x-cmu-raster";
	mt["rast"] = "image/cmu-raster";
	mt["rexx"] = "text/x-script.rexx";
	mt["rf"] = "image/vnd.rn-realflash";
	mt["rgb"] = "image/x-rgb";
	mt["rm"] = "application/vnd.rn-realmedia";
	mt["rm"] = "audio/x-pn-realaudio";
	mt["rmi"] = "audio/mid";
	mt["rmm"] = "audio/x-pn-realaudio";
	mt["rmp"] = "audio/x-pn-realaudio";
	mt["rmp"] = "audio/x-pn-realaudio-plugin";
	mt["rng"] = "application/ringing-tones";
	mt["rng"] = "application/vnd.nokia.ringing-tone";
	mt["rnx"] = "application/vnd.rn-realplayer";
	mt["roff"] = "application/x-troff";
	mt["rp"] = "image/vnd.rn-realpix";
	mt["rpm"] = "audio/x-pn-realaudio-plugin";
	mt["rt"] = "text/richtext";
	mt["rt"] = "text/vnd.rn-realtext";
	mt["rtf"] = "application/rtf";
	mt["rtf"] = "application/x-rtf";
	mt["rtf"] = "text/richtext";
	mt["rtx"] = "application/rtf";
	mt["rtx"] = "text/richtext";
	mt["rv"] = "video/vnd.rn-realvideo";
	mt["s"] = "text/x-asm";
	mt["s3m"] = "audio/s3m";
	mt["saveme"] = "application/octet-stream";
	mt["sbk"] = "application/x-tbook";
	mt["scm"] = "application/x-lotusscreencam";
	mt["scm"] = "text/x-script.guile";
	mt["scm"] = "text/x-script.scheme";
	mt["scm"] = "video/x-scm";
	mt["sdml"] = "text/plain";
	mt["sdp"] = "application/sdp";
	mt["sdp"] = "application/x-sdp";
	mt["sdr"] = "application/sounder";
	mt["sea"] = "application/sea";
	mt["sea"] = "application/x-sea";
	mt["set"] = "application/set";
	mt["sgm"] = "text/sgml";
	mt["sgm"] = "text/x-sgml";
	mt["sgml"] = "text/sgml";
	mt["sgml"] = "text/x-sgml";
	mt["sh"] = "application/x-bsh";
	mt["sh"] = "application/x-sh";
	mt["sh"] = "application/x-shar";
	mt["sh"] = "text/x-script.sh";
	mt["shar"] = "application/x-bsh";
	mt["shar"] = "application/x-shar";
	mt["shtml"] = "text/html";
	mt["shtml"] = "text/x-server-parsed-html";
	mt["sid"] = "audio/x-psid";
	mt["sit"] = "application/x-sit";
	mt["sit"] = "application/x-stuffit";
	mt["skd"] = "application/x-koan";
	mt["skm"] = "application/x-koan";
	mt["skp"] = "application/x-koan";
	mt["skt"] = "application/x-koan";
	mt["sl"] = "application/x-seelogo";
	mt["smi"] = "application/smil";
	mt["smil"] = "application/smil";
	mt["snd"] = "audio/basic";
	mt["snd"] = "audio/x-adpcm";
	mt["sol"] = "application/solids";
	mt["spc"] = "application/x-pkcs7-certificates";
	mt["spc"] = "text/x-speech";
	mt["spl"] = "application/futuresplash";
	mt["spr"] = "application/x-sprite";
	mt["sprite"] = "application/x-sprite";
	mt["src"] = "application/x-wais-source";
	mt["ssi"] = "text/x-server-parsed-html";
	mt["ssm"] = "application/streamingmedia";
	mt["sst"] = "application/vnd.ms-pki.certstore";
	mt["step"] = "application/step";
	mt["stl"] = "application/sla";
	mt["stl"] = "application/vnd.ms-pki.stl";
	mt["stl"] = "application/x-navistyle";
	mt["stp"] = "application/step";
	mt["sv4cpio"] = "application/x-sv4cpio";
	mt["sv4crc"] = "application/x-sv4crc";
	mt["svf"] = "image/vnd.dwg";
	mt["svf"] = "image/x-dwg";
	mt["svr"] = "application/x-world";
	mt["svr"] = "x-world/x-svr";
	mt["swf"] = "application/x-shockwave-flash";
	mt["t"] = "application/x-troff";
	mt["talk"] = "text/x-speech";
	mt["tar"] = "application/x-tar";
	mt["tbk"] = "application/toolbook";
	mt["tbk"] = "application/x-tbook";
	mt["tcl"] = "application/x-tcl";
	mt["tcl"] = "text/x-script.tcl";
	mt["tcsh"] = "text/x-script.tcsh";
	mt["tex"] = "application/x-tex";
	mt["texi"] = "application/x-texinfo";
	mt["texinfo"] = "application/x-texinfo";
	mt["text"] = "application/plain";
	mt["text"] = "text/plain";
	mt["tgz"] = "application/gnutar";
	mt["tgz"] = "application/x-compressed";
	mt["tif"] = "image/tiff";
	mt["tif"] = "image/x-tiff";
	mt["tiff"] = "image/tiff";
	mt["tiff"] = "image/x-tiff";
	mt["tr"] = "application/x-troff";
	mt["tsi"] = "audio/tsp-audio";
	mt["tsp"] = "application/dsptype";
	mt["tsp"] = "audio/tsplayer";
	mt["tsv"] = "text/tab-separated-values";
	mt["turbot"] = "image/florian";
	mt["txt"] = "text/plain";
	mt["uil"] = "text/x-uil";
	mt["uni"] = "text/uri-list";
	mt["unis"] = "text/uri-list";
	mt["unv"] = "application/i-deas";
	mt["uri"] = "text/uri-list";
	mt["uris"] = "text/uri-list";
	mt["ustar"] = "application/x-ustar";
	mt["ustar"] = "multipart/x-ustar";
	mt["uu"] = "application/octet-stream";
	mt["uu"] = "text/x-uuencode";
	mt["uue"] = "text/x-uuencode";
	mt["vcd"] = "application/x-cdlink";
	mt["vcs"] = "text/x-vcalendar";
	mt["vda"] = "application/vda";
	mt["vdo"] = "video/vdo";
	mt["vew"] = "application/groupwise";
	mt["viv"] = "video/vivo";
	mt["viv"] = "video/vnd.vivo";
	mt["vivo"] = "video/vivo";
	mt["vivo"] = "video/vnd.vivo";
	mt["vmd"] = "application/vocaltec-media-desc";
	mt["vmf"] = "application/vocaltec-media-file";
	mt["voc"] = "audio/voc";
	mt["voc"] = "audio/x-voc";
	mt["vos"] = "video/vosaic";
	mt["vox"] = "audio/voxware";
	mt["vqe"] = "audio/x-twinvq-plugin";
	mt["vqf"] = "audio/x-twinvq";
	mt["vql"] = "audio/x-twinvq-plugin";
	mt["vrml"] = "application/x-vrml";
	mt["vrml"] = "model/vrml";
	mt["vrml"] = "x-world/x-vrml";
	mt["vrt"] = "x-world/x-vrt";
	mt["vsd"] = "application/x-visio";
	mt["vst"] = "application/x-visio";
	mt["vsw"] = "application/x-visio";
	mt["w60"] = "application/wordperfect6.0";
	mt["w61"] = "application/wordperfect6.1";
	mt["w6w"] = "application/msword";
	mt["wav"] = "audio/wav";
	mt["wav"] = "audio/x-wav";
	mt["wb1"] = "application/x-qpro";
	mt["wbmp"] = "image/vnd.wap.wbmp";
	mt["web"] = "application/vnd.xara";
	mt["wiz"] = "application/msword";
	mt["wk1"] = "application/x-123";
	mt["wmf"] = "windows/metafile";
	mt["wml"] = "text/vnd.wap.wml";
	mt["wmlc"] = "application/vnd.wap.wmlc";
	mt["wmls"] = "text/vnd.wap.wmlscript";
	mt["wmlsc"] = "application/vnd.wap.wmlscriptc";
	mt["word"] = "application/msword";
	mt["wp"] = "application/wordperfect";
	mt["wp5"] = "application/wordperfect";
	mt["wp5"] = "application/wordperfect6.0";
	mt["wp6"] = "application/wordperfect";
	mt["wpd"] = "application/wordperfect";
	mt["wpd"] = "application/x-wpwin";
	mt["wq1"] = "application/x-lotus";
	mt["wri"] = "application/mswrite";
	mt["wri"] = "application/x-wri";
	mt["wrl"] = "application/x-world";
	mt["wrl"] = "model/vrml";
	mt["wrl"] = "x-world/x-vrml";
	mt["wrz"] = "model/vrml";
	mt["wrz"] = "x-world/x-vrml";
	mt["wsc"] = "text/scriplet";
	mt["wsrc"] = "application/x-wais-source";
	mt["wtk"] = "application/x-wintalk";
	mt["xbm"] = "image/x-xbitmap";
	mt["xbm"] = "image/x-xbm";
	mt["xbm"] = "image/xbm";
	mt["xdr"] = "video/x-amt-demorun";
	mt["xgz"] = "xgl/drawing";
	mt["xif"] = "image/vnd.xiff";
	mt["xl"] = "application/excel";
	mt["xla"] = "application/excel";
	mt["xla"] = "application/x-excel";
	mt["xla"] = "application/x-msexcel";
	mt["xlb"] = "application/excel";
	mt["xlb"] = "application/vnd.ms-excel";
	mt["xlb"] = "application/x-excel";
	mt["xlc"] = "application/excel";
	mt["xlc"] = "application/vnd.ms-excel";
	mt["xlc"] = "application/x-excel";
	mt["xld"] = "application/excel";
	mt["xld"] = "application/x-excel";
	mt["xlk"] = "application/excel";
	mt["xlk"] = "application/x-excel";
	mt["xll"] = "application/excel";
	mt["xll"] = "application/vnd.ms-excel";
	mt["xll"] = "application/x-excel";
	mt["xlm"] = "application/excel";
	mt["xlm"] = "application/vnd.ms-excel";
	mt["xlm"] = "application/x-excel";
	mt["xls"] = "application/excel";
	mt["xls"] = "application/vnd.ms-excel";
	mt["xls"] = "application/x-excel";
	mt["xls"] = "application/x-msexcel";
	mt["xlt"] = "application/excel";
	mt["xlt"] = "application/x-excel";
	mt["xlv"] = "application/excel";
	mt["xlv"] = "application/x-excel";
	mt["xlw"] = "application/excel";
	mt["xlw"] = "application/vnd.ms-excel";
	mt["xlw"] = "application/x-excel";
	mt["xlw"] = "application/x-msexcel";
	mt["xm"] = "audio/xm";
	mt["xml"] = "application/xml";
	mt["xml"] = "text/xml";
	mt["xmz"] = "xgl/movie";
	mt["xpix"] = "application/x-vnd.ls-xpix";
	mt["xpm"] = "image/x-xpixmap";
	mt["xpm"] = "image/xpm";
	mt["x-png"] = "image/png";
	mt["xsr"] = "video/x-amt-showrun";
	mt["xwd"] = "image/x-xwd";
	mt["xwd"] = "image/x-xwindowdump";
	mt["xyz"] = "chemical/x-pdb";
	mt["z"] = "application/x-compress";
	mt["z"] = "application/x-compressed";
	mt["zip"] = "application/x-compressed";
	mt["zip"] = "application/x-zip-compressed";
	mt["zip"] = "application/zip";
	mt["zip"] = "multipart/x-zip";
	mt["zoo"] = "application/octet-stream";
	mt["zsh"] = "text/x-script.zsh";
}
