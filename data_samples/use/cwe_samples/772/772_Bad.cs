
                  SqlConnection conn = new SqlConnection(connString);SqlCommand cmd = new SqlCommand(queryString);cmd.Connection = conn;conn.Open();SqlDataReader rdr = cmd.ExecuteReader();HarvestResults(rdr);conn.Connection.Close();
               
               