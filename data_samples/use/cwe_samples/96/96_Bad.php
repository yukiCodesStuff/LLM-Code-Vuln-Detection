
                  $MessageFile = "messages.out";if ($_GET["action"] == "NewMessage") {$name = $_GET["name"];$message = $_GET["message"];$handle = fopen($MessageFile, "a+");fwrite($handle, "<b>$name</b> says '$message'<hr>\n");fclose($handle);echo "Message Saved!<p>\n";}else if ($_GET["action"] == "ViewMessages") {include($MessageFile);}
               
               