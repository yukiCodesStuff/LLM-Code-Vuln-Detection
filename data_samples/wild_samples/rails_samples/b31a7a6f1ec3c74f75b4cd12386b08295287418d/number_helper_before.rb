        wrap_with_output_safety_handling(number, options.delete(:raise)) {
          ActiveSupport::NumberHelper.public_send(method, number, options)
        }
      end

      def escape_unsafe_delimiters_and_separators(options)
        options[:separator] = ERB::Util.html_escape(options[:separator]) if options[:separator] && !options[:separator].html_safe?
        options[:delimiter] = ERB::Util.html_escape(options[:delimiter]) if options[:delimiter] && !options[:delimiter].html_safe?
        options
      end

      def wrap_with_output_safety_handling(number, raise_on_invalid, &block)
        valid_float = valid_float?(number)
        raise InvalidNumberError, number if raise_on_invalid && !valid_float

        formatted_number = yield

        if valid_float || number.html_safe?
          formatted_number.html_safe
        else
          formatted_number
        end
      end

      def valid_float?(number)
        !parse_float(number, false).nil?
      end

      def parse_float(number, raise_error)
        Float(number)
      rescue ArgumentError, TypeError
        raise InvalidNumberError, number if raise_error
      end
    end
  end
end