const express = require('express');
const app = express()

function GetCredentials(session) {
    return {
        "id": 1,
        "username": "James007",
        "email": "james@ywh.com",
        "session" : "9b13252f346c2073e0c9ed39aad87ba9e9a59dd925606c6cdb12eec0d7368b5b"
     };
}

app.get('/', function (req,res) {
    origin = req.headers["origin"];
    if ( origin == undefined || origin == "" ) {
        console.log("Origin set to wildcard")
        origin = "*";
    }

    const headers = {
        "Access-Control-Allow-Origin": origin,
        "Access-Control-Allow-Credentials": "true",
        "Content-Type": "application/json",
    };
    res.writeHead(200, headers);

    console.log(`[${Date()}][LOG]`, req.headers["referer"], "=>", req.headers["host"]);

    sess = GetCredentials(req.session)

    userCred = {
        "id": sess.id,
        "username": sess.username,
        "email": sess.email,
        "session": sess.session,
    };

    res.end(JSON.stringify(userCred));
});

PORT = 1337
app.listen(PORT, '0.0.0.0', () => {
    console.log(`Server is running on http://0.0.0.0:${PORT}`);
  });
  
