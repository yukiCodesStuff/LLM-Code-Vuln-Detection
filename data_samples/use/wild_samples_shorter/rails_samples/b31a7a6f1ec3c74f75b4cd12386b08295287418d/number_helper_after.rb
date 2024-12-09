      def escape_unsafe_delimiters_and_separators(options)
        options[:separator] = ERB::Util.html_escape(options[:separator]) if options[:separator] && !options[:separator].html_safe?
        options[:delimiter] = ERB::Util.html_escape(options[:delimiter]) if options[:delimiter] && !options[:delimiter].html_safe?
        options[:unit]      = ERB::Util.html_escape(options[:unit]) if options[:unit] && !options[:unit].html_safe?
        options
      end

      def wrap_with_output_safety_handling(number, raise_on_invalid, &block)