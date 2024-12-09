<?php
class Client {
    private $ip;
    private $created;

    function __construct() {
        $this->ip = $_SERVER['HTTP_X_CLIENT_IP'];
        $this->created = date('m/d/Y h:i:s a', time());
        //Code...
    }
    function __wakeup() {
        $data = exec("ping -c 4 -W 5 -n " . $this->ip);
        echo "Result: $data \n";
        //Code...
    }
}

if ( !isset($_COOKIE['client']) || strlen($_COOKIE['client']) == 0 ) {
    $client = new Client();
    setcookie("client", base64_encode(serialize($client)), ['httponly' => true]);
} else {
    $client_data = base64_decode(urldecode($_COOKIE['client']));
    unserialize($client_data);
    //Code...
}

?>