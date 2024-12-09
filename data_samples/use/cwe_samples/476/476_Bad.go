
                  func HandleRequest(client http.Client, request *http.Request) (*http.Response, error) {
                     
                        response, err := client.Do(request)
                        defer response.Body.Close()
                        if err != nil {
                           
                              return nil, err
                           
                        }...
                     }
               
               