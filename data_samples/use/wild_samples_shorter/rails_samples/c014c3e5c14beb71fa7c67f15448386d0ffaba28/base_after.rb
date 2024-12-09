      def execute_callstack_for_multiparameter_attributes(callstack)
        errors = []
        callstack.each do |name, values|
          begin
            klass = (self.class.reflect_on_aggregation(name.to_sym) || column_for_attribute(name)).klass
            if values.empty?
              send(name + "=", nil)
            else
              value = if Time == klass
                instantiate_time_object(name, values)
              elsif Date == klass
                begin
              end

              send(name + "=", value)
            end
          rescue => ex
            errors << AttributeAssignmentError.new("error on assignment #{values.inspect} to #{name}", ex, name)
          end
        end
        unless errors.empty?
          raise MultiparameterAssignmentErrors.new(errors), "#{errors.size} error(s) on assignment of multiparameter attributes"
      end

      def type_cast_attribute_value(multiparameter_name, value)
        multiparameter_name =~ /\([0-9]*([if])\)/ ? value.send("to_" + $1) : value
      end

      def find_parameter_position(multiparameter_name)
        multiparameter_name.scan(/\(([0-9]*).*\)/).first.first