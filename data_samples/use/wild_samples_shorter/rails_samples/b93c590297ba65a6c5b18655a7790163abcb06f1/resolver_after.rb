        query << '{' << ext.map {|e| e && ".#{e}" }.join(',') << ',}'
      end

      query.gsub!(/\{\.html,/, "{.html,.text.html,")
      query.gsub!(/\{\.text,/, "{.text,.text.plain,")

      templates = []
      sanitizer = Hash.new { |h,k| h[k] = Dir["#{File.dirname(k)}/*"] }

      Dir[query].each do |p|
        next if File.directory?(p) || !sanitizer[p].include?(p)

        handler, format = extract_handler_and_format(p, formats)
        contents = File.open(p, "rb") {|io| io.read }

        templates << Template.new(contents, File.expand_path(p), handler,
          :virtual_path => path, :format => format, :updated_at => mtime(p))
      end

      templates
    end

    # Returns the file mtime from the filesystem.
    def mtime(p)