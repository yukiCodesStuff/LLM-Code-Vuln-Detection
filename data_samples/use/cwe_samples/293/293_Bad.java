
                  boolean processConnectionRequest(HttpServletRequest request){String referer = request.getHeader("referer")String trustedReferer = "http://www.example.com/"if(referer.equals(trustedReferer)){openPrivilegedConnection(request);return true;}else{sendPrivilegeError(request);return false;}}
               
               