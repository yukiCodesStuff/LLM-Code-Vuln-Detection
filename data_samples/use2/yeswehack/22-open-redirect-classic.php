<!DOCTYPE html>
<html>
<body>
<b>If you do not redirect click <a style="color:red;" href="/home.html">here</a></b>
<script>
    function filter(s) {
        return s.replace(/[\.@\/]/g, "_")
    }

    var urlParams = new URLSearchParams(window.location.search);
    var path = urlParams.get('r') ?? '';

    if ( path != '' ) {
        location = filter(path);
    }
</script>
</body>
</html>
