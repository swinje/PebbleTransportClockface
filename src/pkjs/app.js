//var myAPIKey = '06bf23b61836a1a86c151642fc30e3e5';

//var utm = require('./utm');

var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);
var messageKeys = require('message_keys');

var redTo='3012430', redFrom='3010011', blueTo='3010011', blueFrom='3012430';

var locationOptions = {
  'timeout': 15000,
  'maximumAge': 60000
};

// Config window closed
Pebble.addEventListener('webviewclosed', function(e) {
  if (e && !e.response) {
    return;
  }

  // Get the keys and values from each config item
  var dict = clay.getSettings(e.response);
  blueTo = dict[messageKeys.BLUETO];
  blueFrom = dict[messageKeys.BLUEFROM];
  redTo = dict[messageKeys.REDTO];
  redFrom = dict[messageKeys.REDFROM];
  
  //console.log("Changed blueTo " + blueTo + " blueFrom " + blueFrom);
  //console.log("Changed redTo " + redTo + " redFrom " + redFrom);

  // Send settings values to watch side
  Pebble.sendAppMessage(dict, function(e) {
    console.log('Sent config data to Pebble');
  }, function(e) {
    console.log('Failed to send config data!');
    console.log(JSON.stringify(e));
  });
});


// At startup
Pebble.addEventListener('ready', function (e) {
  //console.log('connect!' + e.ready);
  window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError,
    locationOptions);
  //console.log(e.type);
});



// When called
Pebble.addEventListener('appmessage', function (e) {
  //console.log('message!');
  window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError,
    locationOptions);
  //console.log(e.type);
  //console.log(JSON.stringify(e.payload));
  //console.log(e.payload['1']);
  //console.log("Fetching blueTo " + blueTo + " blueFrom " + blueFrom);
  //console.log("Fetching redTo " + redTo + " redFrom " + redFrom);
  fetchDeparture(blueFrom, blueTo, 'Blue',1);
  fetchDeparture(redFrom, redTo, 'Red',2);
});

// When closing
Pebble.addEventListener('webviewclosed', function (e) {
  //console.log('webview closed');
  //console.log(e.type);
  //console.log(e.response);
});

// Pulling out time for next stop minus time now and result is hours : minutes to next
var parseFeed = function(data) { 
    var dateNow = new Date();
    // Not sure if the object from ReisAPI is indeed UTC -- seems local Oslo
    var gmt = new Date(data.TravelProposals[0].DepartureTime);
    var utc = gmt.getTime() + (gmt.getTimezoneOffset() * 60000);
    var nextTravel = new Date(utc);
    var diff = new Date(Math.abs(nextTravel - dateNow));
    //var hours = diff.getUTCHours();
    var minutes = diff.getUTCMinutes();
    //return (hours <= 9 ? "0" : "") + hours + ":" + (minutes <= 9 ? "0" : "") + minutes;
    return minutes;
};

// Query the ReisAPI for a departure
function fetchDeparture(sFrom, sTo, sTitle, direction) {
  var req = new XMLHttpRequest();
  var currentdate = new Date(); 
  var currenttime = ('0' + currentdate.getDate()).slice(-2) + "" +
    ('0' + (currentdate.getMonth()+1)).slice(-2) + "" +
    currentdate.getFullYear() + "" +
    ('0' + currentdate.getHours()).slice(-2) + "" +
    ('0' + currentdate.getMinutes()).slice(-2) + "" +
    ('0' + currentdate.getSeconds()).slice(-2);
  var URL = 'http://reisapi.ruter.no/Travel/GetTravels?fromPlace=' + 
    sFrom + '&toPlace=' + sTo +'&isafter=True&time='+currenttime;
  //var URL = 'http://reisapi.ruter.no/Travel/GetTravels?fromPlace=' + 
    //sFrom + '&toPlace=' + sTo +'&isafter=True&time='+currenttime + '&transporttypes=Metro';

  // Fetch data
  //console.log(URL);
  req.open('GET', URL, true);
    
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
        var response = JSON.parse(req.responseText);
        var nextTime= parseFeed(response);
        nextTime = String(nextTime); // force type conversion
        Pebble.sendAppMessage({
          'DIRECTION_KEY': direction,
          'DEPTIME_KEY': nextTime
        });
      } else {
        console.log('Error');
      }
    }
  };
  req.send(null);
}

/*
function iconFromWeatherId(weatherId) {
  if (weatherId < 600) {
    return 2;
  } else if (weatherId < 700) {
    return 3;
  } else if (weatherId > 800) {
    return 1;
  } else {
    return 0;
  }
}
*/

/*function fetchWeather(latitude, longitude) {
  // http://api.met.no/weatherapi/locationforecast/1.9/?lat=60.10;lon=9.58
  var req = new XMLHttpRequest();
  req.open('GET', 'http://api.openweathermap.org/data/2.5/weather?' +
    'lat=' + latitude + '&lon=' + longitude + '&cnt=1&appid=' + myAPIKey, true);
  //console.log(utm.latLon2UTM(latitude, longitude));
  console.log('http://api.openweathermap.org/data/2.5/weather?' +
    'lat=' + latitude + '&lon=' + longitude + '&cnt=1&appid=' + myAPIKey);
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
        console.log(req.responseText);
        var response = JSON.parse(req.responseText);
        var temperature = Math.round(response.main.temp - 273.15);
        var icon = iconFromWeatherId(response.weather[0].id);
        var city = response.name;
        console.log(temperature);
        console.log(icon);
        console.log(city);
        Pebble.sendAppMessage({
          'WEATHER_ICON_KEY': icon,
          'WEATHER_TEMPERATURE_KEY': city + ' ' + temperature + '\xB0C'
        });
      } else {
        console.log('Error');
        Pebble.sendAppMessage({
          'WEATHER_TEMPERATURE_KEY': req.status + ' (E)'
        });
      }
    }
  };
  req.send(null);
}*/

function fetchWeather(latitude, longitude) {
  var req = new XMLHttpRequest();
  req.open('GET', 'http://api.met.no/weatherapi/locationforecast/1.9/?' +
    'lat=' + latitude + '&lon=' + longitude, true);
   //console.log('http://api.met.no/weatherapi/locationforecast/1.9/?' +
   // 'lat=' + latitude + '&lon=' + longitude);
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
        // Since parsing XML is not supported
        var response =req.responseText;
        var dictionary = response.split("\n");
        var lines = dictionary.length;
        var i;

        // Pick out the temperature
        for (i = 0; i < lines; i++) 
          if (dictionary[i].match(/<temperature id="TTT"/))
              break;
        var found = dictionary[i].match(/value="(.*)"/i);
        var temperature = (Math.round(parseFloat(found[1]))).toString();
        
        // Pick out the icon
        for (i = i; i < lines; i++)
          if (dictionary[i].match(/<symbol id="/))
              break;
        // Pick out the icon
        found = dictionary[i].match(/number="(.*)"/i);
        var icon = found[1];
        //console.log("icon " + icon);

        Pebble.sendAppMessage({
          'WEATHER_ICON_KEY': icon,
          'WEATHER_TEMPERATURE_KEY': temperature + '\xB0C'
        });
      } else {
        console.log('Error');
        Pebble.sendAppMessage({
          'WEATHER_TEMPERATURE_KEY': req.status + ' (E)'
        });
      }
    }
  };
  req.send(null);
}



function locationSuccess(pos) {
  var coordinates = pos.coords;
  fetchWeather(coordinates.latitude, coordinates.longitude);
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  Pebble.sendAppMessage({
    'WEATHER_TEMPERATURE_KEY': 'N/A'
  });
}





