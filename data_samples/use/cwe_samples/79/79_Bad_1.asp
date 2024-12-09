
                  <%protected System.Web.UI.WebControls.Label EmployeeName;...string query = "select * from emp where id=" + eid;sda = new SqlDataAdapter(query, conn);sda.Fill(dt);string name = dt.Rows[0]["Name"];...EmployeeName.Text = name;%><p><asp:label id="EmployeeName" runat="server" /></p>
               
               