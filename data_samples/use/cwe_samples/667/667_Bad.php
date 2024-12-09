
                  function writeToLog($message){$logfile = fopen("logFile.log", "a");
                        //attempt to get logfile lock
                        if (flock($logfile, LOCK_EX)) {fwrite($logfile,$message);
                           // unlock logfile
                           flock($logfile, LOCK_UN);}else {print "Could not obtain lock on logFile.log, message not recorded\n";}}fclose($logFile);
               
               