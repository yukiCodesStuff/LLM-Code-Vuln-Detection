<?php

function resetHash($con, $email, $reset) {
    if ( $reset ) {
        $hs = NULL;
    } else {
        $hs = md5(random_int(1000, 9999));
    }

    $q = $con->prepare("UPDATE `users` SET resethash=? WHERE email=?");
    $q->bind_param("ss", $hs, $email);
    $q->execute();
    
    return $hs;
}

function ChangePasswd($con, $passwd, $email, $hs) {
    if ( strlen($passwd) < 8 ) {
        return false;
    } 
    $q = $con->prepare("UPDATE users SET password=? WHERE resethash IS NOT NULL AND (email=? AND resethash=?)");
    $q->bind_param("sss", $passwd, $email, $hs);
    $q->execute();

    return $q->affected_rows;
}

$hash = isset($_GET['hash']) ? $_GET['hash'] : "";
$email = isset($_GET['email']) ? $_GET['email'] : "";

if ( filter_var($email, FILTER_VALIDATE_EMAIL) && isset($_GET['new'])) {
    resetHash($mysqlDB, $email, false);
    echo "<br>The user with this email has received a new password reset hash!<br>";
    //Code... Sent `$hash` to the users mail (reset link)
}

if ( isset($_POST['passwd']) && ChangePasswd($mysqlDB,$_POST['passwd'],$email,$hash) ) {
    resetHash($mysqlDB, $email, true);
    echo "<br>Password changed!<br>";
}

$mysqlDB->close();

?>

<!DOCTYPE html>
<html>
<body>
<div>
    <form method="GET">
        <labal>Reset your password</label>
        <input type="email" name="email" placeholder="Ex: example@gmail.com">
        <input type="submit">
    </form>
</div>
<body>
</html>
