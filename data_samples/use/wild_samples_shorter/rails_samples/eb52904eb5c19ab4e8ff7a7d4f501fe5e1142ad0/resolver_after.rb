      locals = locals.map(&:to_s).sort!.freeze

      cached(key, [name, prefix, partial], details, locals) do
        find_templates(name, prefix, partial, details, locals)
      end
    end

    alias :find_all_anywhere :find_all
    deprecate :find_all_anywhere

    def find_all_with_query(query) # :nodoc:
      @cache.cache_query(query) { find_template_paths(File.join(@path, query)) }
    end
    # This is what child classes implement. No defaults are needed
    # because Resolver guarantees that the arguments are present and
    # normalized.
    def find_templates(name, prefix, partial, details, locals = [])
      raise NotImplementedError, "Subclasses must implement a find_templates(name, prefix, partial, details, locals = []) method"
    end

    # Handles templates caching. If a key is given and caching is on
    # always check the cache before hitting the resolver. Otherwise,

    private

      def find_templates(name, prefix, partial, details, locals)
        path = Path.build(name, prefix, partial)
        query(path, details, details[:formats], locals)
      end

      def query(path, details, formats, locals)
        template_paths = find_template_paths_from_details(path, details)
        template_paths = reject_files_external_to_app(template_paths)

        template_paths.map do |template|
          build_template(template, path.virtual, locals)
        end
  # The same as FileSystemResolver but does not allow templates to store
  # a virtual path since it is invalid for such resolvers.
  class FallbackFileSystemResolver < FileSystemResolver #:nodoc:
    private_class_method :new

    def self.instances
      [new(""), new("/")]
    end

    def build_template(template, virtual_path, locals)
      super(template, nil, locals)
    end

    def reject_files_external_to_app(files)
      files
    end
  end
end