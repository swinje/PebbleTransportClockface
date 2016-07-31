
// At startup
Pebble.addEventListener('ready', function (e) {
  console.log('connect!' + e.ready);
  console.log(e.type);
});

// When called
Pebble.addEventListener('appmessage', function (e) {
  console.log('message!');
  console.log(e.type);
  console.log(JSON.stringify(e.payload));
  console.log(e.payload['1']);
  fetchDeparture('3012430', '3010011', 'Hov',1);
  fetchDeparture('3010011', '3012430', 'JBT',2);
});

// When closing
Pebble.addEventListener('webviewclosed', function (e) {
  console.log('webview closed');
  console.log(e.type);
  console.log(e.response);
});

// Pulling out time for next stop minus time now and result is hours : minutes to next
var parseFeed = function(data) { 
    var dateNow = new Date();
    // Not sure if the object from ReisAPI is indeed UTC -- seems local Oslo
    var gmt = new Date(data.TravelProposals[0].DepartureTime);
    var utc = gmt.getTime() + (gmt.getTimezoneOffset() * 60000);
    var nextTravel = new Date(utc);
    var diff = new Date(Math.abs(nextTravel - dateNow));
    var hours = diff.getUTCHours();
    var minutes = diff.getUTCMinutes();
    return (hours <= 9 ? "0" : "") + hours + ":" + (minutes <= 9 ? "0" : "") + minutes;
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
    sFrom + '&toPlace=' + sTo +'&isafter=True&time='+currenttime + '&transporttypes=Metro';

  // Fetch data
  req.open('GET', URL, true);
    
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
        var response = JSON.parse(req.responseText);
        var nextTime= parseFeed(response);
        nextTime = sTitle + " " + nextTime;
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
