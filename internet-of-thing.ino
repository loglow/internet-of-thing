// internet-of-thing
// Daniel Gilbert
// loglow@gmail.com
// Copyright 2015



//=============================================================================
// INCLUDES
//=============================================================================



#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>



//=============================================================================
// DEFINES
//=============================================================================



// Define used GPIO pins
#define RELAY_PIN 2



//=============================================================================
// GLOBALS
//=============================================================================



// Base name of the device
String device_name = "Switch";

// Create webserver object
ESP8266WebServer server(80);

// Stores current state of relay
uint8_t switch_on = 0;



//=============================================================================
// SETUP
//=============================================================================



void setup() {

	// Setup and init GPIO pins
	pinMode(RELAY_PIN, OUTPUT);
	digitalWrite(RELAY_PIN, LOW);

	// Build unique device name from base name
	// and the last 6 chars of the mac address
	String s = WiFi.macAddress();
	s.replace(":", "");
	device_name += "-"+s.substring(6);

	// Setup and start the webserver
	server.on("/", serve_root);
	server.on("/style.css", serve_style_css);
	server.on("/normalize.css", serve_normalize_css);
	server.begin();
	
	// Enable the device-created access point
	// with no password, aka open network
	WiFi.softAP(device_name.c_str());

	// Attempt to reconnect to the last-known wireless network
	// SSID and password are stored in non-volatile memory
	sta_connect("", "");
}



//=============================================================================
// MAIN LOOP
//=============================================================================



void loop() {

	// Process any pending webserver actions
	server.handleClient();
}



//=============================================================================
// FUNCTIONS
//=============================================================================



// Attempt to connect to the specified wireless network
// If ssid is empty then fall back to whatever was used last
// If unable to connect then disable STA (client) mode entirely
void sta_connect(String ssid, String password) {
	if(!ssid) WiFi.begin();
	else WiFi.begin(ssid.c_str(), password.c_str());
	if(WiFi.waitForConnectResult() != WL_CONNECTED) WiFi.disconnect(true);
}



// Convert an IPAddress object (array) into a printable string
String ip_to_s(IPAddress a) {
    char s[16];
    sprintf(s, "%u.%u.%u.%u", a[0], a[1], a[2], a[3]);
    return String(s);
}



// Serve the primary HTML needed for the web interface
// Begin by parsing and acting on any POST data from user interaction
void serve_root() {

	// Turn the relay on or off
	if(server.arg("switch") == "1") {
		digitalWrite(RELAY_PIN, HIGH);
		switch_on = 1;
	} else if(server.arg("switch") == "0") {
		digitalWrite(RELAY_PIN, LOW);
		switch_on = 0;
	}

	// Enable or disable either of the two wireless networking modes
	else if(server.arg("ap") == "1") WiFi.softAP(device_name.c_str());
	else if(server.arg("ap") == "0") WiFi.softAPdisconnect(true);
	else if(server.arg("sta") == "1") sta_connect(server.arg("ssid"), server.arg("password"));
	else if(server.arg("sta") == "0") WiFi.disconnect(true);

	// Get and store info about the current config
	String ssid = WiFi.SSID();
	String password = WiFi.psk();
	String soft = ip_to_s(WiFi.softAPIP());
	String local = ip_to_s(WiFi.localIP());
	String gateway = ip_to_s(WiFi.gatewayIP());
	
	// Assemble HTML into one big string
	String s =
		"<!DOCTYPE html>\n"
		"<html>\n"
		"\t<head>\n"
		"\t\t<title>"+device_name+"</title>\n"
		"\t\t<meta name='viewport' content='width=device-width'>\n"
		"\t\t<link rel='stylesheet' type='text/css' href='style.css'>\n"
		"\t</head>\n"
		"\t<body>\n"
		"\t\t<form action='/' method='post'>\n"
		"\t\t\t<table>\n"
		"\t\t\t\t<tr><th colspan='2'>"+device_name+"</th></tr>\n";
	if(switch_on) s += // Relay is ON
		"\t\t\t\t<tr><th colspan='2' class='on'>On</th></tr>\n"
		"\t\t\t\t<tr><th colspan='2'><button name='switch' value='0'>Turn Off</button></th></tr>\n";
	else s += // Relay is OFF
		"\t\t\t\t<tr><th colspan='2' class='off'>Off</th></tr>\n"
		"\t\t\t\t<tr><th colspan='2'><button name='switch' value='1'>Turn On</button></th></tr>\n";
	s +=
		"\t\t\t\t<tr><th colspan='2'><a href='/'>Refresh</a></th></tr>\n"
		"\t\t\t\t<tr><th colspan='2' class='head'>Access Point</th></tr>\n";
	if(WiFi.getMode() & WIFI_AP) s += // AP is ON
		"\t\t\t\t<tr><td class='left'>Enabled</td><td>Yes</td></tr>\n"
		"\t\t\t\t<tr><td class='left'>SSID</td><td>"+device_name+"</td></tr>\n"
		"\t\t\t\t<tr><td class='left'>IP Address</td><td>"+soft+"</td></tr>\n"
		"\t\t\t\t<tr><th colspan='2'><button name='ap' value='0'>Disable</button></th></tr>\n";
	else s += // AP is OFF
		"\t\t\t\t<tr><td class='left'>Enabled</td><td>No</td></tr>\n"
		"\t\t\t\t<tr><th colspan='2'><button name='ap' value='1'>Enable</button></th></tr>\n";
	s +=
		"\t\t\t\t<tr><th colspan='2' class='head'>Client Mode</th></tr>\n";
	if(WiFi.status() == WL_CONNECTED) s += // STA is connected
		"\t\t\t\t<tr><td class='left'>Status</td><td>Connected</td></tr>\n"
		"\t\t\t\t<tr><td class='left'>SSID</td><td>"+ssid+"</td></tr>\n"
		"\t\t\t\t<tr><td class='left'>Password</td><td>"+password+"</td></tr>\n"
		"\t\t\t\t<tr><td class='left'>IP Address</td><td>"+local+"</td></tr>\n"
		"\t\t\t\t<tr><td class='left'>Gateway</td><td>"+gateway+"</td></tr>\n"
		"\t\t\t\t<tr><th colspan='2'><button name='sta' value='0'>Disconnect</button></th></tr>\n";
	else s += // STA is NOT connected
		"\t\t\t\t<tr><td class='left'>Status</td><td>Disconnected</td></tr>\n"
		"\t\t\t\t<tr><td class='left'>SSID</td><td><input type='text' name='ssid'></td></tr>\n"
		"\t\t\t\t<tr><td class='left'>Password</td><td><input type='text' name='password'></td></tr>\n"
		"\t\t\t\t<tr><th colspan='2'><button name='sta' value='1'>Connect</button></th></tr>\n";
	s +=
		"\t\t\t</table>\n"
		"\t\t</form>\n"
		"\t</body>\n"
		"</html>";

	// Send HTML document to the client
	server.send(200, "text/html", s);
}



// Serve the custom CSS needed for the web interface
void serve_style_css() {

	// Assemble CSS into one big string
	String s =
		"@import 'normalize.css';\n"
		"html{font-family:arial;white-space:nowrap;}\n"
		"body{background:#eee;}\n"
		"table{width:100%;}\n"
		"th,td{border:1px solid #999;padding:10px;font-size:18px;color:#555;background:white;width:50%;}\n"
		"th{font-size:32px;}\n"
		".head{background:#999;color:white;font-size:22px;}\n"
		".left{text-align:right;font-weight:bold;}\n"
		".on{background:#7c7;color:white;}\n"
		".off{background:#c77;color:white;}\n"
		"a,button{text-decoration:none;background:#69f;color:white;padding:4px 8px 3px 8px;border:0;border-radius:8px;display:inline-block;}\n"
		"a:hover,button:hover{background:#47d;}\n"
		"a:active,button:active{background:#25b;}\n"
		"input[type='text']{border:3px solid #69f;border-radius:8px;width:120px;}";

	// Send CSS document to the client
	server.send(200, "text/css", s);
}



// Serve the minified normalize CSS document needed for the web interface
// This script ensures ensures basic consistency between different web browsers
// About: normalize.css v3.0.3 | MIT License | github.com/necolas/normalize.css
// Unmodified from original except for double quote symbols changed to single quotes
void serve_normalize_css() {
	String s = "html{font-family:sans-serif;-ms-text-size-adjust:100%;-webkit-text-size-adjust:100%}body{margin:0}article,aside,details,figcaption,figure,footer,header,hgroup,main,menu,nav,section,summary{display:block}audio,canvas,progress,video{display:inline-block;vertical-align:baseline}audio:not([controls]){display:none;height:0}[hidden],template{display:none}a{background-color:transparent}a:active,a:hover{outline:0}abbr[title]{border-bottom:1px dotted}b,strong{font-weight:bold}dfn{font-style:italic}h1{font-size:2em;margin:.67em 0}mark{background:#ff0;color:#000}small{font-size:80%}sub,sup{font-size:75%;line-height:0;position:relative;vertical-align:baseline}sup{top:-0.5em}sub{bottom:-0.25em}img{border:0}svg:not(:root){overflow:hidden}figure{margin:1em 40px}hr{box-sizing:content-box;height:0}pre{overflow:auto}code,kbd,pre,samp{font-family:monospace,monospace;font-size:1em}button,input,optgroup,select,textarea{color:inherit;font:inherit;margin:0}button{overflow:visible}button,select{text-transform:none}button,html input[type='button'],input[type='reset'],input[type='submit']{-webkit-appearance:button;cursor:pointer}button[disabled],html input[disabled]{cursor:default}button::-moz-focus-inner,input::-moz-focus-inner{border:0;padding:0}input{line-height:normal}input[type='checkbox'],input[type='radio']{box-sizing:border-box;padding:0}input[type='number']::-webkit-inner-spin-button,input[type='number']::-webkit-outer-spin-button{height:auto}input[type='search']{-webkit-appearance:textfield;box-sizing:content-box}input[type='search']::-webkit-search-cancel-button,input[type='search']::-webkit-search-decoration{-webkit-appearance:none}fieldset{border:1px solid silver;margin:0 2px;padding:.35em .625em .75em}legend{border:0;padding:0}textarea{overflow:auto}optgroup{font-weight:bold}table{border-collapse:collapse;border-spacing:0}td,th{padding:0}";
	server.send(200, "text/css", s);
}



// EOF
