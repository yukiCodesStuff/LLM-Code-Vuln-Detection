<?php
function GetNonce() {
    return base64_encode(random_bytes(20));
}

$nonce = GetNonce();
header("Content-Security-Policy: script-src 'nonce-$nonce'");
?>

<!DOCTYPE html>
<html>
<body>
<p><?= ( isset($_GET["message"]) ) ? $_GET["message"] : 'session expired' ?></p>

<input type="hidden" id="host" value="<?= $_SERVER['HTTP_HOST'] ?>">

<script nonce="<?= $nonce ?>">
   const URLparams = new URLSearchParams(window.location.search);
   const logout = URLparams.get('logout') || false;
    if ( logout ) {
        const host = document.querySelector('#host').value;
        const scrptTag = document.createElement('script');

        scrptTag.nonce = document.currentScript.nonce;
        scrptTag.src = ("//" + host + '/checkLogout.js');

        document.body.appendChild(scrptTag);
    }
</script>
</body>
</html>
