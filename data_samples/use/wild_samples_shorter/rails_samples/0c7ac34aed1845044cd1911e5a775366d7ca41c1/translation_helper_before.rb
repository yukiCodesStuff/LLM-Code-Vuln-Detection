require 'action_view/helpers/tag_helper'
require 'i18n/exceptions'

module I18n
  class ExceptionHandler
    include Module.new {
      def call(exception, locale, key, options)
        exception.is_a?(MissingTranslation) && options[:rescue_format] == :html ? super.html_safe : super
      end
    }
  end
end

module ActionView
  # = Action View Translation Helpers
  module Helpers
    module TranslationHelper
      # Delegates to <tt>I18n#translate</tt> but also performs three additional functions.
      #
      # First, it'll pass the <tt>rescue_format: :html</tt> option to I18n so that any
      # thrown +MissingTranslation+ messages will be turned into inline spans that
      #
      #   * have a "translation-missing" class set,
      #   * contain the missing key as a title attribute and
      #   * a titleized version of the last key segment as a text.
      # naming convention helps to identify translations that include HTML tags so that
      # you know what kind of output to expect when you call translate in a template.
      def translate(key, options = {})
        options.merge!(:rescue_format => :html) unless options.key?(:rescue_format)
        options[:default] = wrap_translate_defaults(options[:default]) if options[:default]
        if html_safe_translation_key?(key)
          html_safe_options = options.dup
          options.except(*I18n::RESERVED_KEYS).each do |name, value|
            unless name == :count && value.is_a?(Numeric)
        else
          I18n.translate(scope_key_by_partial(key), options)
        end
      end
      alias :t :translate

      # Delegates to <tt>I18n.localize</tt> with no additional functionality.