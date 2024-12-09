
                  $role = $_COOKIES['role'];if (!$role) {$role = getRole('user');if ($role) {// save the cookie to send out in future responsessetcookie("role", $role, time()+60*60*2);}else{ShowLoginScreen();die("\n");}}if ($role == 'Reader') {DisplayMedicalHistory($_POST['patient_ID']);}else{die("You are not Authorized to view this record\n");}
               
               