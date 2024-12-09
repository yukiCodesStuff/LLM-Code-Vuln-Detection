
                  try {openDbConnection();}
                     //print exception message that includes exception message and configuration file location
                     catch (Exception $e) {echo 'Caught exception: ', $e->getMessage(), '\n';echo 'Check credentials in config file at: ', $Mysql_config_location, '\n';}
               
               