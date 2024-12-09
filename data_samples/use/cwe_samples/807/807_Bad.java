
                  Cookie[] cookies = request.getCookies();for (int i =0; i< cookies.length; i++) {Cookie c = cookies[i];if (c.getName().equals("role")) {userRole = c.getValue();}}
               
            