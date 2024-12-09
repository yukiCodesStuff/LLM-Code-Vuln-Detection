      find_all(*args).first || raise(MissingTemplate.new(self, *args))
    end

    def find_all(path, prefixes = [], *args)
      prefixes = [prefixes] if String === prefixes
      prefixes.each do |prefix|
        paths.each do |resolver|
          templates = resolver.find_all(path, prefix, *args)
          return templates unless templates.empty?
        end
      end
      []
    end

    def exists?(path, prefixes, *args)
      find_all(path, prefixes, *args).any?

    private

    def typecast(paths)
      paths.map do |path|
        case path
        when Pathname, String