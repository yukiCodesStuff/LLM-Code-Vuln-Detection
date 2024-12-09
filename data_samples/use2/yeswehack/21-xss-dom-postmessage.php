<!DOCTYPE html>
<html>
<body>

<center>
<label>LABEL</label><br>
<textarea type="text" id="username" placeholder="your username..."></textarea><br>
<input id="btn" type="submit" value="START">

<p id="out">Connected to game server address: </p>
</center>
<script>
  console.log("Waiting for connection...")
  window.addEventListener("message", (event)=>{
    console.log("[OK] Connected!")
    address = event.data
    document.getElementById('out').innerHTML += `<u>${address}</u>`
  });
  //Code...
</script>
</body>
</html>
