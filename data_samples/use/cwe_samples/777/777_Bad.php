
                  $dir = "/home/cwe/languages";$lang = $_GET['lang'];if (preg_match("/[A-Za-z0-9]+/", $lang)) {include("$dir/$lang");}else {echo "You shall not pass!\n";}
               
               