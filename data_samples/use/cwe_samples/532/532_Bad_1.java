
                  locationClient = new LocationClient(this, this, this);locationClient.connect();currentUser.setLocation(locationClient.getLastLocation());
                     ...
                     
                     catch (Exception e) {AlertDialog.Builder builder = new AlertDialog.Builder(this);builder.setMessage("Sorry, this application has experienced an error.");AlertDialog alert = builder.create();alert.show();Log.e("ExampleActivity", "Caught exception: " + e + " While on User:" + User.toString());}
               
               