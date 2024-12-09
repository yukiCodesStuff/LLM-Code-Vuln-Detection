
                  <%Statement stmt = conn.createStatement();ResultSet rs = stmt.executeQuery("select * from emp where id="+eid);if (rs != null) {rs.next();String name = rs.getString("name");}%>
                     Employee Name: <%= name %>
               
               