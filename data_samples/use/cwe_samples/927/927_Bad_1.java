
                  Intent intent = new Intent();intent.setAction("com.example.service.UserExists");intent.putExtra("Username", uname_string);sendStickyBroadcast(intent);
               
               