
                  locationClient = new LocationClient(this, this, this);locationClient.connect();Location userCurrLocation;userCurrLocation = locationClient.getLastLocation();setTimeZone(userCurrLocation);
               
               