
      def delegate_number_helper_method(method, number, options)
        return unless number
        options = escape_unsafe_delimiters_and_separators(options.symbolize_keys)

        wrap_with_output_safety_handling(number, options.delete(:raise)) {
          ActiveSupport::NumberHelper.public_send(method, number, options)
        }
      end

      def escape_unsafe_delimiters_and_separators(options)
        options[:separator] = ERB::Util.html_escape(options[:separator]) if options[:separator] && !options[:separator].html_safe?
        options[:delimiter] = ERB::Util.html_escape(options[:delimiter]) if options[:delimiter] && !options[:delimiter].html_safe?
        options[:unit]      = ERB::Util.html_escape(options[:unit]) if options[:unit] && !options[:unit].html_safe?
        options
      end

      def wrap_with_output_safety_handling(number, raise_on_invalid, &block)
        valid_float = valid_float?(number)
        raise InvalidNumberError, number if raise_on_invalid && !valid_float
