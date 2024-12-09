
                  Intent intent = new Intent();intent.setAction("com.example.BackupUserData");intent.setData(file_uri);intent.addFlags(FLAG_GRANT_READ_URI_PERMISSION);sendBroadcast(intent);
               
               