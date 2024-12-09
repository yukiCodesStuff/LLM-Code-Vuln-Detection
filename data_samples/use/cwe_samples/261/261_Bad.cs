
                  ...string value = regKey.GetValue(passKey).ToString();byte[] decVal = Convert.FromBase64String(value);NetworkCredential netCred = newNetworkCredential(username,decVal.toString(),domain);...
               
               