        query << '{' << ext.map {|e| e && ".#{e}" }.join(',') << ',}'
      end

      Dir[query].reject { |p| File.directory?(p) }.map do |p|
        handler, format = extract_handler_and_format(p, formats)

        contents = File.open(p, "rb") {|io| io.read }

        Template.new(contents, File.expand_path(p), handler,
          :virtual_path => path, :format => format, :updated_at => mtime(p))
      end
    end

    # Returns the file mtime from the filesystem.
    def mtime(p)