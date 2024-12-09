        key = "#{path}.#{action_name}#{key}"
      end

      ActiveSupport::HtmlSafeTranslation.translate(key, **options)
    end
    alias :t :translate

    # Delegates to `I18n.localize`.
      I18n.localize(object, **options)
    end
    alias :l :localize
  end
end