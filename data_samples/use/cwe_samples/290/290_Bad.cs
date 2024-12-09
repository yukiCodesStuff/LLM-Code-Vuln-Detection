
                  IPAddress hostIPAddress = IPAddress.Parse(RemoteIpAddress);IPHostEntry hostInfo = Dns.GetHostByAddress(hostIPAddress);if (hostInfo.HostName.EndsWith("trustme.com")) {trusted = true;}
               
               