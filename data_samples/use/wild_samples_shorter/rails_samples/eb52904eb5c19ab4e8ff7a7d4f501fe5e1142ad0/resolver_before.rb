      locals = locals.map(&:to_s).sort!.freeze

      cached(key, [name, prefix, partial], details, locals) do
        find_templates(name, prefix, partial, details, false, locals)
      end
    end

    def find_all_anywhere(name, prefix, partial = false, details = {}, key = nil, locals = [])
      locals = locals.map(&:to_s).sort!.freeze

      cached(key, [name, prefix, partial], details, locals) do
        find_templates(name, prefix, partial, details, true, locals)
      end
    end

    def find_all_with_query(query) # :nodoc:
      @cache.cache_query(query) { find_template_paths(File.join(@path, query)) }
    end
    # This is what child classes implement. No defaults are needed
    # because Resolver guarantees that the arguments are present and
    # normalized.
    def find_templates(name, prefix, partial, details, outside_app_allowed = false, locals = [])
      raise NotImplementedError, "Subclasses must implement a find_templates(name, prefix, partial, details, outside_app_allowed = false, locals = []) method"
    end

    # Handles templates caching. If a key is given and caching is on
    # always check the cache before hitting the resolver. Otherwise,

    private

      def find_templates(name, prefix, partial, details, outside_app_allowed = false, locals)
        path = Path.build(name, prefix, partial)
        query(path, details, details[:formats], outside_app_allowed, locals)
      end

      def query(path, details, formats, outside_app_allowed, locals)
        template_paths = find_template_paths_from_details(path, details)
        template_paths = reject_files_external_to_app(template_paths) unless outside_app_allowed

        template_paths.map do |template|
          build_template(template, path.virtual, locals)
        end
  # The same as FileSystemResolver but does not allow templates to store
  # a virtual path since it is invalid for such resolvers.
  class FallbackFileSystemResolver < FileSystemResolver #:nodoc:
    def self.instances
      [new(""), new("/")]
    end

    def build_template(template, virtual_path, locals)
      super(template, nil, locals)
    end
  end
end