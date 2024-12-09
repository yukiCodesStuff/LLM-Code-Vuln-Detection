
                  <% for (Iterator i = guestbook.iterator(); i.hasNext(); ) {Entry e = (Entry) i.next(); %><p>Entry #<%= e.getId() %></p><p><%= e.getText() %></p><%} %>
                  
               
            