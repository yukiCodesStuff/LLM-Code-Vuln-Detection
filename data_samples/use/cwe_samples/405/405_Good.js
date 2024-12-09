
               var test_string = "Bad characters: $@#";
               var good_pattern  = /^((?=(\w+))\2\s?)*$/i;
               var result = test_string.search(good_pattern);
             
             