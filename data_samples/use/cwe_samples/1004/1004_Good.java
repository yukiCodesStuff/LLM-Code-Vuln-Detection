
                  String sessionID = generateSessionId();Cookie c = new Cookie("session_id", sessionID);c.setHttpOnly(true);response.addCookie(c);
               
            