      end
    end

    def find_all_anywhere(name, prefix, partial=false, details={}, key=nil, locals=[])
      cached(key, [name, prefix, partial], details, locals) do
        find_templates(name, prefix, partial, details, true)
      end
    end

    def find_all_with_query(query) # :nodoc:
      @cache.cache_query(query) { find_template_paths(File.join(@path, query)) }
    end


    private

    def find_templates(name, prefix, partial, details, outside_app_allowed = false)
      path = Path.build(name, prefix, partial)
      query(path, details, details[:formats], outside_app_allowed)
    end

    def query(path, details, formats, outside_app_allowed)
      query = build_query(path, details)

      template_paths = find_template_paths(query)
      template_paths = reject_files_external_to_app(template_paths) unless outside_app_allowed

      template_paths.map do |template|
        handler, format, variant = extract_handler_and_format_and_variant(template, formats)
        contents = File.binread(template)
      end
    end

    def reject_files_external_to_app(files)
      files.reject { |filename| !inside_path?(@path, filename) }
    end

    def find_template_paths(query)
      Dir[query].reject do |filename|
        File.directory?(filename) ||
          # deals with case-insensitive file systems.
      end
    end

    def inside_path?(path, filename)
      filename = File.expand_path(filename)
      path = File.join(path, '')
      filename.start_with?(path)
    end

    # Helper for building query glob string based on resolver's pattern.
    def build_query(path, details)
      query = @pattern.dup
