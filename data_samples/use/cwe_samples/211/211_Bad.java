
                  public void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
                        String username = request.getParameter("username");
                           
                           // May cause unchecked NullPointerException.
                           if (username.length() < 10) {...}
                     }
               
            