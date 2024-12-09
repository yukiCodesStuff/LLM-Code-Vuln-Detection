
                  IntentFilter filter = new IntentFilter("com.example.RemoveUser");MyReceiver receiver = new MyReceiver();registerReceiver(receiver, filter);
                     public class DeleteReceiver extends BroadcastReceiver {@Overridepublic void onReceive(Context context, Intent intent) {int userID = intent.getIntExtra("userID");destroyUserData(userID);}}
               
               