
                  
                     // iOS
                     -(BOOL) webView:(UIWebView *)exWebView shouldStartLoadWithRequest:(NSURLRequest *)exRequest navigationType:(UIWebViewNavigationType)exNavigationType{
                        NSURL *URL = [exRequest URL];if ([[URL scheme] isEqualToString:@"exampleScheme"]){
                              NSString *functionString = [URL resourceSpecifier];if ([functionString hasPrefix:@"specialFunction"]){
                                    
                                       
                                       // Make data available back in webview.
                                       UIWebView *webView = [self writeDataToView:[URL query]];
                                 }return NO;
                           }return YES;
                     }
               
               