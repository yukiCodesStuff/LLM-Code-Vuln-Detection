
                  public class WebViewGUI extends Activity {
                        WebView mainWebView;
                           public void onCreate(Bundle savedInstanceState) {super.onCreate(savedInstanceState);mainWebView = new WebView(this);mainWebView.getSettings().setJavaScriptEnabled(true);mainWebView.addJavascriptInterface(new JavaScriptInterface(), "userInfoObject");mainWebView.loadUrl("file:///android_asset/www/index.html");setContentView(mainWebView);}
                           final class JavaScriptInterface {
                              JavaScriptInterface () {}
                                 public String getUserInfo() {return currentUser.Info();}
                           }
                     }
               
               