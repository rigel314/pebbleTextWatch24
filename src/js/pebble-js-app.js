var AskReqVal = 1;
var SetReqVal = 2;
var initialDate = 1;
var initialOh = 1;
var lastConfig;

Pebble.addEventListener("ready", function()
{
	console.log("ready called!");
	setTimeout(askAppMsg, 500);
});

function askAppMsg()
{
	console.log("askAppMsg() called!");
	Pebble.sendAppMessage({ "req": AskReqVal });
}

function resendAppMsg()
{
	console.log("resendAppMsg() called!");
	Pebble.sendAppMessage(lastConfig);
}

Pebble.addEventListener("appmessage", function(e)
{
	console.log("Received message: " + JSON.stringify(e.payload));
	if (e.payload['err'])
	{
		setTimeout(resendAppMsg, 500);
	}
	else
	{
		initialDate = e.payload['date'];
		initialOh = e.payload['oh'];
	}
});

Pebble.addEventListener("showConfiguration", function()
{
	var thishtml;

	console.log("showing configuration");

	var url = "http://rigel314.github.io/pebbleTextWatch24/config.html" + "?datevar=" + (initialDate==1) ? "1" : "" + "&ohvar=" + initialOh;

	console.log("url: " + url);

	Pebble.openURL(url);
});

Pebble.addEventListener("webviewclosed", function(e)
{
	var options = JSON.parse(decodeURIComponent(e.response));

	console.log("configuration closed");
	console.log("Options = " + e.response);

	lastConfig = { "req": SetReqVal, "date": options['dateVar'], "oh": ((options['ohVar1']) ? 0 : 1) };
	Pebble.sendAppMessage(lastConfig);
	setTimeout(askAppMsg, 500);
});
