          self.content_type = Mime::JS
        end

        "/**/#{options[:callback]}(#{json})"
      else
        self.content_type ||= Mime::JSON
        json
      end