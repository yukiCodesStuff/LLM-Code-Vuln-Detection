
                        ...
                            for (i=0; i<NB_SUBORDINATE; i++)
                            begin
                                for (j=0; j<NB_MANAGER; j++)
                                begin
                                    assign connectivity_map_o[i][j] = access_ctrl_i[i][j][priv_lvl_i];
                            end
                        end
                        ...
                    
                