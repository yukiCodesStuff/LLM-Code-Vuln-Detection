
                  
                     
                     ...
                     IntentFilter filter = new IntentFilter("com.example.URLHandler.openURL");MyReceiver receiver = new MyReceiver();registerReceiver(receiver, filter);
                     ...
                     
                     public class UrlHandlerReceiver extends BroadcastReceiver {
                        @Overridepublic void onReceive(Context context, Intent intent) {
                              if("com.example.URLHandler.openURL".equals(intent.getAction())) {String URL = intent.getStringExtra("URLToOpen");int length = URL.length();
                                 
                                 ...
                                 }
                           }
                     }
               
               