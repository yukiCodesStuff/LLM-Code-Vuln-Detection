
                  IntentFilter filter = new IntentFilter("com.example.service.UserExists");MyReceiver receiver = new MyReceiver();registerReceiver(receiver, filter);
               
               