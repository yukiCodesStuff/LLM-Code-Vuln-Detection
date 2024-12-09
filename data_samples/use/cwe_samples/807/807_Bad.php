
                  $auth = $_COOKIES['authenticated'];if (! $auth) {if (AuthenticateUser($_POST['user'], $_POST['password']) == "success") {// save the cookie to send out in future responsessetcookie("authenticated", "1", time()+60*60*2);}else {ShowLoginScreen();die("\n");}}DisplayMedicalHistory($_POST['patient_ID']);
               
               