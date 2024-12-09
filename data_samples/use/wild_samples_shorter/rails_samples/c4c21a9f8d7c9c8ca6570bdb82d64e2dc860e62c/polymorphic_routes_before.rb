
            args = []

            route = record_list.map { |parent|
              case parent
              when Symbol, String
                parent.to_s
              when Class
                args << parent
                parent.model_name.singular_route_key
              else
                args << parent.to_model
                parent.to_model.model_name.singular_route_key
              end
            }

            route <<
            case record
            when Symbol, String
              record.to_s
            when Class
              @key_strategy.call record.model_name
            else
              model = record.to_model