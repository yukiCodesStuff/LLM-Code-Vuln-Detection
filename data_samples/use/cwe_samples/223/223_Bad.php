
                  function login($userName,$password){
                        if(authenticate($userName,$password)){return True;}else{incrementLoginAttempts($userName);if(recentLoginAttempts($userName) > 5){writeLog("Failed login attempt by User: " . $userName . " at " + date('r') );}}
                     }
               
               