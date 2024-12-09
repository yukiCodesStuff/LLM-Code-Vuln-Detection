      def deserialize_argument(argument)
        case argument
        when String
          argument
        when *PERMITTED_TYPES
          argument
        when Array
          argument.map { |arg| deserialize_argument(arg) }