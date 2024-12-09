      TRAILING_STAR_REGEXP = /^(text|application)\/\*/
      # all media-type parameters need to be before the q-parameter
      # https://www.rfc-editor.org/rfc/rfc7231#section-5.3.2
      PARAMETER_SEPARATOR_REGEXP = /;\s*q="?/
      ACCEPT_HEADER_REGEXP = /[^,\s"](?:[^,"]|"[^"]*")*/

      def register_callback(&block)
        @register_callbacks << block
      def parse(accept_header)
        if !accept_header.include?(",")
          if (index = accept_header.index(PARAMETER_SEPARATOR_REGEXP))
            accept_header = accept_header[0, index].strip
          end
          return [] if accept_header.blank?
          parse_trailing_star(accept_header) || Array(Mime::Type.lookup(accept_header))
        else