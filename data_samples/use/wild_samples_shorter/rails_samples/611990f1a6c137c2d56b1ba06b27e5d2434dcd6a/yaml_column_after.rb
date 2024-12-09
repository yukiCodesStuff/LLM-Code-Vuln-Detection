          raise ArgumentError, "Cannot serialize #{object_class}. Classes passed to `serialize` must have a 0 argument constructor."
        end

        def yaml_load(payload)
          if !ActiveRecord.use_yaml_unsafe_load
            YAML.safe_load(payload, permitted_classes: ActiveRecord.yaml_column_permitted_classes, aliases: true)
          else
            if YAML.respond_to?(:unsafe_load)
              YAML.unsafe_load(payload)
            else
              YAML.load(payload)
            end
          end
        end
    end
  end