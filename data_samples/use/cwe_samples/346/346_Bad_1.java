
                  
                     // Android
                     @Overridepublic boolean shouldOverrideUrlLoading(WebView view, String url){
                        if (url.substring(0,14).equalsIgnoreCase("examplescheme:")){if(url.substring(14,25).equalsIgnoreCase("getUserInfo")){writeDataToView(view, UserData);return false;}else{return true;}}
                     }
               
               