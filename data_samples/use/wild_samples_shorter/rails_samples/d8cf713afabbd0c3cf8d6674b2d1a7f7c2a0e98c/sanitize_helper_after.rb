      #   strip_tags("<div id='top-bar'>Welcome to my website!</div>")
      #   # => Welcome to my website!
      def strip_tags(html)
        self.class.full_sanitizer.sanitize(html)
      end

      # Strips all link tags from +text+ leaving just the link text.
      #