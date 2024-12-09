
      def delegate_number_helper_method(method, number, options)
        return unless number
        options = escape_unsafe_options(options.symbolize_keys)

        wrap_with_output_safety_handling(number, options.delete(:raise)) {
          ActiveSupport::NumberHelper.public_send(method, number, options)
        }
      end

      def escape_unsafe_options(options)
        options[:format]          = ERB::Util.html_escape(options[:format]) if options[:format]
        options[:negative_format] = ERB::Util.html_escape(options[:negative_format]) if options[:negative_format]
        options[:separator]       = ERB::Util.html_escape(options[:separator]) if options[:separator]
        options[:delimiter]       = ERB::Util.html_escape(options[:delimiter]) if options[:delimiter]
        options[:unit]            = ERB::Util.html_escape(options[:unit]) if options[:unit] && !options[:unit].html_safe?
        options[:units]           = escape_units(options[:units]) if options[:units] && Hash === options[:units]
        options
      end

      def escape_units(units)
        Hash[units.map do |k, v|
          [k, ERB::Util.html_escape(v)]
        end]
      end

      def wrap_with_output_safety_handling(number, raise_on_invalid, &block)
        valid_float = valid_float?(number)
        raise InvalidNumberError, number if raise_on_invalid && !valid_float
