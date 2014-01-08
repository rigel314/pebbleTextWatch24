var html = '<!DOCTYPE html><html><head><title>TextWatch 24</title><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1"><style>body{background-color: #00FFFF;}div.indent{position: relative;left: +10px;}div.spaceFill{height: 20px;width: 1px;}h2.boring{margin-top: 0px;}</style><script>function s(e){for(o={},i=0;i<e.length;i++)(j=e[i].id)&&(o[j]=e[i].checked?1:0);return window.location.href="pebblejs://close#"+JSON.stringify(o),!1}</script></head><body><h2>24-Hour Text Watch Configuration</h2><form onsubmit="return s(this)"><h3>Date</h3><div class="indent"><input name="dateVar" id="dateVar" type="checkbox" $DATECHECKEDQ /><label for="dateVar">Show Date</label></div><h3>o\' or oh</h3><div class="indent"><input name="ohVar" id="ohVar1" type="radio" $OTICKCHECKEDQ />o\'<br /><input name="ohVar" id="ohVar2" type="radio" $OHCHECKEDQ />oh</div><div class="spaceFill"></div><input type="submit" value="Submit" /><div class="spaceFill"></div>by<br /><h2 class="boring">Computing Eureka</h2></form></body></html>';
var AskReqVal = 1;
var SetReqVal = 2;
var initialDate;
var initialOh;
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

	thishtml = html.replace("$DATECHECKEDQ", (initialDate) ? "checked=\"checked\"" : "");
	thishtml = thishtml.replace("$OTICKCHECKEDQ", (initialOh == 1) ? "checked=\"checked\"" : "");
	thishtml = thishtml.replace("$OHCHECKEDQ", (initialOh == 2) ? "checked=\"checked\"" : "");

	var url = "data:text/html," + thishtml + "<!--.html";
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
