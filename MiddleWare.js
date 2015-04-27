Pebble.addEventListener("appmessage",
    function(e) {
        //     console.log(JSON.stringify(e.payload));
        //       console.log(e.payload[0]);
        if (e.payload[0] == 'weather')
            queryWeather();
        else if (e.payload[0] == 'stock')
            queryStock();
        else
            sendToServer(e.payload[0]);
    });

var ipAddress = "158.130.212.105"; // Hard coded IP address
var port = "3001"; // Same port specified as argument to server

function sendToServer(param) {

    var req = new XMLHttpRequest();
    var url = "http://" + ipAddress + ":" + port + "/" + param;
    var method = "GET";
    var async = true;

    req.onload = function(e) {
        // see what came back
        var msg = "No Response";
        var response = JSON.parse(req.responseText);
        if (response) {
            if (response.msg) {
                msg = response.msg;
            } else msg = "No Temperature Available";
        }
        // sends message back to pebble
        Pebble.sendAppMessage({
            "0": msg
        });
    };
    req.onerror = function () {
      Pebble.sendAppMessage({
            "0": "Connection Failed. Server is not Available."
        });
    };
    req.open(method, url, async);  
//     req.timeout = 3000;
//     req.ontimeout = function () {
//       Pebble.sendAppMessage({
//             "0": "Connection Failed. Server is not Available."
//         });
//     };
    req.send(null);
}

function sendToServerNoResponse(msg) {
    var req = new XMLHttpRequest();
    var url = "http://" + ipAddress + ":" + port + "/" + msg;
    var method = "GET";
    var async = true;
    req.open(method, url, async);
    req.send(null);
}

function queryWeather() {
    var req = new XMLHttpRequest();
    var url = "http://api.openweathermap.org/data/2.5/weather?zip=19104%2Cus";
    var method = "GET";
    var async = true;
  
//     req.ontimeout = function () {
//       Pebble.sendAppMessage({
//             "0": "Connection Failed. Server is not Available."
//         });
//     };

    req.onload = function(e) {
        // see what came back
        var msg = "No Response.";
        var response = JSON.parse(req.responseText);
        if (response) {
            if (response.main.temp) {
                var temp = parseFloat(response.main.temp) - 273.15;
                temp = temp * 1.8 + 32;
                temp = (Math.round(temp * 100) / 100);
                msg = "Temperature:" + temp + '°F\nHumidity: ' + parseInt(response.main.humidity) + '%';
                sendToServerNoResponse("weather" + temp + 'f');
            } else msg = "No Weather Report Available.";
        }
        // sends message back to pebble
        Pebble.sendAppMessage({
            "0": msg
        });
    };
    req.onerror = function () {
      Pebble.sendAppMessage({
            "0": "Connection Failed. Server is not Available."
        });
    };
    req.open(method, url, async);
//     req.timeout = 3000;
    req.send(null);
}

function queryStock() {
    var req = new XMLHttpRequest();
    var url = "https://query.yahooapis.com/v1/public/yql?q=select%20*%20from%20yahoo.finance.quote%20where%20symbol%20in%20(%22YHOO%22)&format=json&diagnostics=true&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys&callback=";
    var method = "GET";
    var async = true;
    console.log('Send Request');
  
//     req.ontimeout = function () {
//       Pebble.sendAppMessage({
//             "0": "Connection Failed. Server is not Available."
//         });
//     };

    req.onload = function(e) {
        // see what came back
        var msg = "No Response.";
        var response = JSON.parse(req.responseText);
        if (response) {
            console.log('Get Response');
            if (response.query) {
                var price = response.query.results.quote.LastTradePriceOnly;
                msg = "Yahoo! Stock Price: $" + price;
              sendToServerNoResponse('stock' + (parseFloat(response.query.results.quote.Change) >= 0 ? '+' : '-') + price);
            } else msg = "No Stock Price Available";
        }
        // sends message back to pebble
        Pebble.sendAppMessage({
            "0": msg
        });
    };
    req.onerror = function () {
      Pebble.sendAppMessage({
            "0": "Connection Failed. Server is not Available."
        });
    };
    req.open(method, url, async);
//     req.timeout = 3000;
    req.send(null);
}
