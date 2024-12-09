
            args = []

            route = record_list.map do |parent|
              case parent
              when Symbol
                parent.to_s
              when String
                raise(ArgumentError, "Please use symbols for polymorphic route arguments.")
              when Class
                args << parent
                parent.model_name.singular_route_key
              else
                args << parent.to_model
                parent.to_model.model_name.singular_route_key
              end
            end

            route <<
            case record
            when Symbol
              record.to_s
            when String
              raise(ArgumentError, "Please use symbols for polymorphic route arguments.")
            when Class
              @key_strategy.call record.model_name
            else
              model = record.to_model