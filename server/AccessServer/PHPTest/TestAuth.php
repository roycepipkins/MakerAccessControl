<?php
// Pull in the NuSOAP code
require_once('nusoap/nusoap.php');
// Create the server instance
$server = new soap_server;
// Register the method to expose
$server->register('authRequest');

function authRequest($devAddr, $id)
{
	//note that for testing purposes this function ignores the devAddr
	//We really don't care which door the user is trying to get into.
	
	
	$users = parse_ini_file('/usr/local/lib/idlist.ini');
	
	if (array_key_exists($id, $users))
	{
		$authReply = array(
			'accessGranted' => 1, 
			'msgLine1' => 'Welcome     ',
			'msgLine2' => substr($users[$id], 0, 16)
		);
	}
	else
	{
		$authReply = array(
			'accessGranted' => 0, 
			'msgLine1' => '     Access     ',
			'msgLine2' => '     Denied!    '
		);
	}
	
	return $authReply;
}

$HTTP_RAW_POST_DATA = isset($HTTP_RAW_POST_DATA) ? $HTTP_RAW_POST_DATA : '';
$server->service($HTTP_RAW_POST_DATA);

?>