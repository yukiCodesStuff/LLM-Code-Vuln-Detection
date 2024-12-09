
                  ...string userName = ctx.getAuthenticatedUserName();string query = "SELECT * FROM items WHERE owner = '" + userName + "' AND itemname = '" + ItemName.Text + "'";sda = new SqlDataAdapter(query, conn);DataTable dt = new DataTable();sda.Fill(dt);...
               
               