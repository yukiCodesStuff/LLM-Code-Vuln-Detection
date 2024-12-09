
                 let sessionId = generateSessionId()
                 let cookieOptions = { domain: 'example.com', sameSite: 'Strict' }
                 response.cookie('sessionid', sessionId, cookieOptions)
               
            