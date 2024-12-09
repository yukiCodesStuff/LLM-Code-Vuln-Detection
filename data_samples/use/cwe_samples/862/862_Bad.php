
                  function runEmployeeQuery($dbName, $name){mysql_select_db($dbName,$globalDbHandle) or die("Could not open Database".$dbName);
                        //Use a prepared statement to avoid CWE-89
                        $preparedStatement = $globalDbHandle->prepare('SELECT * FROM employees WHERE name = :name');$preparedStatement->execute(array(':name' => $name));return $preparedStatement->fetchAll();}
                     /.../
                     
                     $employeeRecord = runEmployeeQuery('EmployeeDB',$_GET['EmployeeName']);
               
               