
                  String address = request.getParameter("address");address = address.trim();String updateString = "UPDATE shippingInfo SET address='?' WHERE email='cwe@example.com'";emailAddress = con.prepareStatement(updateString);emailAddress.setString(1, address);
               
               