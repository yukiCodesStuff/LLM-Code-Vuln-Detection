          raise ArgumentError, "Cannot serialize #{object_class}. Classes passed to `serialize` must have a 0 argument constructor."
        end

        if YAML.respond_to?(:unsafe_load)
          def yaml_load(payload)
            YAML.unsafe_load(payload)
          end
        else
          def yaml_load(payload)
            YAML.load(payload)
          end
        end
    end
  end