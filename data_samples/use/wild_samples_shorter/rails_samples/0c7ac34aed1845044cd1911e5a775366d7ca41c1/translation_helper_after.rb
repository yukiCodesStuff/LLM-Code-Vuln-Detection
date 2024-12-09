require 'action_view/helpers/tag_helper'
require 'i18n/exceptions'

module ActionView
  # = Action View Translation Helpers
  module Helpers
    module TranslationHelper
      # Delegates to <tt>I18n#translate</tt> but also performs three additional functions.
      #
      # First, it will ensure that any thrown +MissingTranslation+ messages will be turned 
      # into inline spans that:
      #
      #   * have a "translation-missing" class set,
      #   * contain the missing key as a title attribute and
      #   * a titleized version of the last key segment as a text.
      # naming convention helps to identify translations that include HTML tags so that
      # you know what kind of output to expect when you call translate in a template.
      def translate(key, options = {})
        options[:default] = wrap_translate_defaults(options[:default]) if options[:default]

        # If the user has specified rescue_format then pass it all through, otherwise use
        # raise and do the work ourselves
        options[:raise] = true unless options.key?(:raise) || options.key?(:rescue_format)
        if html_safe_translation_key?(key)
          html_safe_options = options.dup
          options.except(*I18n::RESERVED_KEYS).each do |name, value|
            unless name == :count && value.is_a?(Numeric)
        else
          I18n.translate(scope_key_by_partial(key), options)
        end
      rescue I18n::MissingTranslationData => e
        keys = I18n.normalize_keys(e.locale, e.key, e.options[:scope])
        content_tag('span', keys.last.to_s.titleize, :class => 'translation_missing', :title => "translation missing: #{keys.join('.')}")
      end
      alias :t :translate

      # Delegates to <tt>I18n.localize</tt> with no additional functionality.