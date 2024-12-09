
                  Intent intent = new Intent();intent.setAction("com.example.CreateUser");intent.putExtra("Username", uname_string);intent.putExtra("Password", pw_string);sendBroadcast(intent);
               
               