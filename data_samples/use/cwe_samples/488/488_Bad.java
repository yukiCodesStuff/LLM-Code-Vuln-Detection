
                  public class GuestBook extends HttpServlet {
                        String name;
                           protected void doPost (HttpServletRequest req, HttpServletResponse res) {name = req.getParameter("name");...out.println(name + ", thanks for visiting!");}
                     }
               
               