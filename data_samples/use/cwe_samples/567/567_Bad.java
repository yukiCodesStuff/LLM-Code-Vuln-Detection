
                  public static class Counter extends HttpServlet {static int count = 0;protected void doGet(HttpServletRequest in, HttpServletResponse out)throws ServletException, IOException {out.setContentType("text/plain");PrintWriter p = out.getWriter();count++;p.println(count + " hits so far!");}}
               
               