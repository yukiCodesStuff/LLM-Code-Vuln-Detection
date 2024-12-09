<?php
function showImage($imageName) {
    return base64_encode(file_get_contents($imageName));
}
function UploadImage($f) : string {
    if ( !isset($f) || preg_match('/\.(jpg|jpeg|png)/', $f["name"]) !== 1) {
        echo "Invalid image given!";
        die();
    }
    $file = "uploads/".basename($f["name"]);

    if ( !move_uploaded_file($f["tmp_name"], $file) ) {
        echo "Failed! (unkown)";
    }
    return $file;
}

if( isset($_POST["submit"])) {
    $imageName = UploadImage($_FILES["profileImage"]);
    echo "New profile picture \"".htmlspecialchars($imageName)."\" has been successfully uploaded!";
} else {
    $imageName = "uploads/default.jpg";
}
?>

<html>
<body>
<img id="profilePicture" src="data:image/png;base64,<?= showImage($imageName) ?>">
<div>
    <form action="" method="POST" enctype="multipart/form-data">
        <label for="upload">Upload your profile picture! (jpeg/png)</label>
        <input type="file" id="profileImage" name="profileImage">
        <input type="submit" value="Submit" name="submit">
    </form>
</div>
<body>
</html>
