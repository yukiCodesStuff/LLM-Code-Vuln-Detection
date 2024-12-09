
                  public class RedirectServlet extends HttpServlet {
                     
                        protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {String query = request.getQueryString();if (query.contains("url")) {String url = request.getParameter("url");response.sendRedirect(url);}}
                     }
               
               