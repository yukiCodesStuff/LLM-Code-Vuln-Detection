
                  
                     
                     ...
                     IntentFilter filter = new IntentFilter(Intent.ACTION_SHUTDOWN);BroadcastReceiver sReceiver = new ShutDownReceiver();registerReceiver(sReceiver, filter);
                     ...
                     
                     public class ShutdownReceiver extends BroadcastReceiver {@Overridepublic void onReceive(final Context context, final Intent intent) {mainActivity.saveLocalData();mainActivity.stopActivity();}}
               
               