
                        ...
                        assign key_big0    = debug_mode_i ? 192'b0 : {key0[0],
                        key0[1], key0[2], key0[3], key0[4], key0[5]};
                        
                        assign key_big1    = debug_mode_i ? 192'b0 : {key1[0],
                        key1[1], key1[2], key1[3], key1[4], key1[5]};
                        
                        assign key_big2    = {key2[0], key2[1], key2[2],
                        key2[3], key2[4], key2[5]};
                        ...
                        assign key_big = key_sel[1] ? key_big2 : ( key_sel[0] ?
                        key_big1 : key_big0 );
                        ...
                    
                    