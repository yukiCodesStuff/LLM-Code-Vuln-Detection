      end
    end

    def find_all_with_query(query) # :nodoc:
      @cache.cache_query(query) { find_template_paths(File.join(@path, query)) }
    end


    private

    def find_templates(name, prefix, partial, details)
      path = Path.build(name, prefix, partial)
      query(path, details, details[:formats])
    end

    def query(path, details, formats)
      query = build_query(path, details)

      template_paths = find_template_paths(query)

      template_paths.map do |template|
        handler, format, variant = extract_handler_and_format_and_variant(template, formats)
        contents = File.binread(template)
      end
    end

    def find_template_paths(query)
      Dir[query].reject do |filename|
        File.directory?(filename) ||
          # deals with case-insensitive file systems.
      end
    end

    # Helper for building query glob string based on resolver's pattern.
    def build_query(path, details)
      query = @pattern.dup
