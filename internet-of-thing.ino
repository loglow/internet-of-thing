#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define RELAY_PIN 2

String device_name = "Switch";

ESP8266WebServer server(80);

uint8_t switch_on = 0;

void setup() {
	pinMode(RELAY_PIN, OUTPUT);
	digitalWrite(RELAY_PIN, LOW);

	String s = WiFi.macAddress();
	s.replace(":", "");
	device_name += "-"+s.substring(6);

	setup_server();
	WiFi.softAP(device_name.c_str());
	sta_connect("", "");
}

void loop() {
	server.handleClient();
}

void setup_server() {
	server.on("/", serve_root);
	server.on("/style.css", serve_style_css);
	server.on("/normalize.css", serve_normalize_css);
	server.begin();
}

void sta_connect(String ssid, String password) {
	if(!ssid) WiFi.begin();
	else WiFi.begin(ssid.c_str(), password.c_str());
	if(WiFi.waitForConnectResult() != WL_CONNECTED) WiFi.disconnect(true);
}

String ip_to_s(IPAddress a) {
    char s[16];
    sprintf(s, "%u.%u.%u.%u", a[0], a[1], a[2], a[3]);
    return String(s);
}

void serve_root() {

	if(server.arg("switch") == "1") {
		digitalWrite(RELAY_PIN, HIGH);
		switch_on = 1;
	}
	else if(server.arg("switch") == "0") {
		digitalWrite(RELAY_PIN, LOW);
		switch_on = 0;
	}
	else if(server.arg("ap") == "1") WiFi.softAP(device_name.c_str());
	else if(server.arg("ap") == "0") WiFi.softAPdisconnect(true);
	else if(server.arg("sta") == "1") sta_connect(server.arg("ssid"), server.arg("password"));
	else if(server.arg("sta") == "0") WiFi.disconnect(true);

	String ssid = WiFi.SSID();
	String password = WiFi.psk();
	String soft = ip_to_s(WiFi.softAPIP());
	String local = ip_to_s(WiFi.localIP());
	String gateway = ip_to_s(WiFi.gatewayIP());
	
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
	if(switch_on) s +=
		"\t\t\t\t<tr><th colspan='2' class='on'>On</th></tr>\n"
		"\t\t\t\t<tr><th colspan='2'><button name='switch' value='0'>Turn Off</button></th></tr>\n";
	else s +=
		"\t\t\t\t<tr><th colspan='2' class='off'>Off</th></tr>\n"
		"\t\t\t\t<tr><th colspan='2'><button name='switch' value='1'>Turn On</button></th></tr>\n";
	s +=
		"\t\t\t\t<tr><th colspan='2'><a href='/'>Refresh</a></th></tr>\n"
		"\t\t\t\t<tr><th colspan='2' class='head'>Access Point</th></tr>\n";
	if(WiFi.getMode() & WIFI_AP) s +=
		"\t\t\t\t<tr><td class='left'>Enabled</td><td>Yes</td></tr>\n"
		"\t\t\t\t<tr><td class='left'>SSID</td><td>"+device_name+"</td></tr>\n"
		"\t\t\t\t<tr><td class='left'>IP Address</td><td>"+soft+"</td></tr>\n"
		"\t\t\t\t<tr><th colspan='2'><button name='ap' value='0'>Disable</button></th></tr>\n";
	else s +=
		"\t\t\t\t<tr><td class='left'>Enabled</td><td>No</td></tr>\n"
		"\t\t\t\t<tr><th colspan='2'><button name='ap' value='1'>Enable</button></th></tr>\n";
	s +=
		"\t\t\t\t<tr><th colspan='2' class='head'>Client Mode</th></tr>\n";
	if(WiFi.status() == WL_CONNECTED) s +=
		"\t\t\t\t<tr><td class='left'>Status</td><td>Connected</td></tr>\n"
		"\t\t\t\t<tr><td class='left'>SSID</td><td>"+ssid+"</td></tr>\n"
		"\t\t\t\t<tr><td class='left'>Password</td><td>"+password+"</td></tr>\n"
		"\t\t\t\t<tr><td class='left'>IP Address</td><td>"+local+"</td></tr>\n"
		"\t\t\t\t<tr><td class='left'>Gateway</td><td>"+gateway+"</td></tr>\n"
		"\t\t\t\t<tr><th colspan='2'><button name='sta' value='0'>Disconnect</button></th></tr>\n";
	else s +=
		"\t\t\t\t<tr><td class='left'>Status</td><td>Disconnected</td></tr>\n"
		"\t\t\t\t<tr><td class='left'>SSID</td><td><input type='text' name='ssid'></td></tr>\n"
		"\t\t\t\t<tr><td class='left'>Password</td><td><input type='text' name='password'></td></tr>\n"
		"\t\t\t\t<tr><th colspan='2'><button name='sta' value='1'>Connect</button></th></tr>\n";
	s +=
		"\t\t\t</table>\n"
		"\t\t</form>\n"
		"\t</body>\n"
		"</html>";

	server.send(200, "text/html", s);
}

void serve_style_css() {
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
		"input[type=text]{border:3px solid #69f;border-radius:8px;width:120px;}";
	server.send(200, "text/css", s);
}

void serve_normalize_css() {
	String s = "/*! normalize.css v3.0.3 | MIT License | github.com/necolas/normalize.css */html{font-family:sans-serif;-ms-text-size-adjust:100%;-webkit-text-size-adjust:100%}body{margin:0}article,aside,details,figcaption,figure,footer,header,hgroup,main,menu,nav,section,summary{display:block}audio,canvas,progress,video{display:inline-block;vertical-align:baseline}audio:not([controls]){display:none;height:0}[hidden],template{display:none}a{background-color:transparent}a:active,a:hover{outline:0}abbr[title]{border-bottom:1px dotted}b,strong{font-weight:bold}dfn{font-style:italic}h1{font-size:2em;margin:.67em 0}mark{background:#ff0;color:#000}small{font-size:80%}sub,sup{font-size:75%;line-height:0;position:relative;vertical-align:baseline}sup{top:-0.5em}sub{bottom:-0.25em}img{border:0}svg:not(:root){overflow:hidden}figure{margin:1em 40px}hr{box-sizing:content-box;height:0}pre{overflow:auto}code,kbd,pre,samp{font-family:monospace,monospace;font-size:1em}button,input,optgroup,select,textarea{color:inherit;font:inherit;margin:0}button{overflow:visible}button,select{text-transform:none}button,html input[type=\"button\"],input[type=\"reset\"],input[type=\"submit\"]{-webkit-appearance:button;cursor:pointer}button[disabled],html input[disabled]{cursor:default}button::-moz-focus-inner,input::-moz-focus-inner{border:0;padding:0}input{line-height:normal}input[type=\"checkbox\"],input[type=\"radio\"]{box-sizing:border-box;padding:0}input[type=\"number\"]::-webkit-inner-spin-button,input[type=\"number\"]::-webkit-outer-spin-button{height:auto}input[type=\"search\"]{-webkit-appearance:textfield;box-sizing:content-box}input[type=\"search\"]::-webkit-search-cancel-button,input[type=\"search\"]::-webkit-search-decoration{-webkit-appearance:none}fieldset{border:1px solid silver;margin:0 2px;padding:.35em .625em .75em}legend{border:0;padding:0}textarea{overflow:auto}optgroup{font-weight:bold}table{border-collapse:collapse;border-spacing:0}td,th{padding:0}";
	server.send(200, "text/css", s);
}