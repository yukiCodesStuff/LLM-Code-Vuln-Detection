<!DOCTYPE html>
<html>
<title><?= $title ?></title>
<body>
<h1><?= $title ?></h1>

<center>
<form method="GET">
  <label for="email">Subscribe to our newsletter</label><br>
  <input type="text" name="email" value=""><br>
  <input type="submit" value="Subscribe">
</form>

<span id="out"></span>
</center>

<script>
  out = document.getElementById('out');

  email = '<?php echo htmlspecialchars($_GET['email']);?>';

  if ( email.split("@").length == 2  ) {
    console.log(email)
    out.innerHTML = 'Welcome: <b>'+email+'</b>';
  }

</script>

<div>
<?= $design ?>
</div>
</body>
</html>
