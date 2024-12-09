
                  Intent intent = new Intent();intent.setAction("com.example.OpenURL");intent.putExtra("URL_TO_OPEN", url_string);sendOrderedBroadcastAsUser(intent);
               
               