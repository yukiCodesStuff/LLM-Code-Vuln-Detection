        key = "#{path}.#{action_name}#{key}"
      end

      if options[:default]
        options[:default] = [options[:default]] unless options[:default].is_a?(Array)
        options[:default] = options[:default].map do |value|
          value.is_a?(String) ? ERB::Util.html_escape(value) : value
        end
      end

      if options[:raise].nil?
        options[:default] = [] unless options[:default]
        options[:default] << MISSING_TRANSLATION
      end

      result = ActiveSupport::HtmlSafeTranslation.translate(key, **options)

      if result == MISSING_TRANSLATION
        +"translation missing: #{key}"
      else
        result
      end
    end
    alias :t :translate

    # Delegates to `I18n.localize`.
      I18n.localize(object, **options)
    end
    alias :l :localize

    private
      MISSING_TRANSLATION = -(2**60)
      private_constant :MISSING_TRANSLATION
  end
end