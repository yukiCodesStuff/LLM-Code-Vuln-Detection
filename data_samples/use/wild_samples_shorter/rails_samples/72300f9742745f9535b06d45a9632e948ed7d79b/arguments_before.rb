      def deserialize_argument(argument)
        case argument
        when String
          GlobalID::Locator.locate(argument) || argument
        when *PERMITTED_TYPES
          argument
        when Array
          argument.map { |arg| deserialize_argument(arg) }