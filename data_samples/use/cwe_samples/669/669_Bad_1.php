
                  
                     
                     // API flag, output JSON if set
                     $json = $_GET['json']$username = $_GET['user']if(!$json){
                        $record = getUserRecord($username);foreach($record as $fieldName => $fieldValue){
                              if($fieldName == "email_address") {
                                    
                                       
                                       // skip displaying user emails
                                       continue;
                                 }else{writeToHtmlPage($fieldName,$fieldValue);}
                           }
                     }else{$record = getUserRecord($username);echo json_encode($record);}
               
               