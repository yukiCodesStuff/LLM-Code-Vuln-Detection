
                  ...
		  conn = new SqlConnection(_ConnectionString);
		  conn.Open();
		  int16 id = System.Convert.ToInt16(invoiceID.Text);
		  SqlCommand query = new SqlCommand( "SELECT * FROM invoices WHERE id = @id", conn);
		  query.Parameters.AddWithValue("@id", id);
		  SqlDataReader objReader = objCommand.ExecuteReader();
		  ...
		  
               
               