
                  use CGI qw(:standard);
                     sub config_file_add_key {
                        my ($fname, $key, $arg) = @_;
                           
                           # code to add a field/key to a file goes here
                           
                        
                     }
                     sub config_file_set_key {
                        my ($fname, $key, $arg) = @_;
                           
                           # code to set key to a particular file goes here
                           
                        
                     }
                     sub config_file_delete_key {
                        my ($fname, $key, $arg) = @_;
                           
                           # code to delete key from a particular file goes here
                           
                        
                     }
                     sub handleConfigAction {
                        my ($fname, $action) = @_;my $key = param('key');my $val = param('val');
                           
                           # this is super-efficient code, especially if you have to invoke
                           
                           
                           
                           # any one of dozens of different functions!
                           
                           my $code = "config_file_$action_key(\$fname, \$key, \$val);";eval($code);
                     }
                     $configfile = "/home/cwe/config.txt";print header;if (defined(param('action'))) {handleConfigAction($configfile, param('action'));}else {print "No action specified!\n";}
               
               