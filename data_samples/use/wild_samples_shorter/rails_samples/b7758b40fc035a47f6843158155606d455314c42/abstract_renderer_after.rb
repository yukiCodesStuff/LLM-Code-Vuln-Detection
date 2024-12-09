  # that new object is called in turn. This abstracts the setup and rendering
  # into a separate classes for partials and templates.
  class AbstractRenderer #:nodoc:
    delegate :find_template, :find_file, :template_exists?, :with_fallbacks, :with_layout_format, :formats, :to => :@lookup_context

    def initialize(lookup_context)
      @lookup_context = lookup_context
    end