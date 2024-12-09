      find_all(*args).first || raise(MissingTemplate.new(self, *args))
    end

    def find_file(path, prefixes = [], *args)
      _find_all(path, prefixes, args, true).first || raise(MissingTemplate.new(self, path, prefixes, *args))
    end

    def find_all(path, prefixes = [], *args)
      _find_all path, prefixes, args, false
    end

    def exists?(path, prefixes, *args)
      find_all(path, prefixes, *args).any?

    private

    def _find_all(path, prefixes, args, outside_app)
      prefixes = [prefixes] if String === prefixes
      prefixes.each do |prefix|
        paths.each do |resolver|
          if outside_app
            templates = resolver.find_all_anywhere(path, prefix, *args)
          else
            templates = resolver.find_all(path, prefix, *args)
          end
          return templates unless templates.empty?
        end
      end
      []
    end

    def typecast(paths)
      paths.map do |path|
        case path
        when Pathname, String