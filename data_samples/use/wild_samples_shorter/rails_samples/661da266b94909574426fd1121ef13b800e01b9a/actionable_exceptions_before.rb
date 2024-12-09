
    private
      def actionable_request?(request)
        request.show_exceptions? && request.post? && request.path == endpoint
      end

      def redirect_to(location)
        body = "<html><body>You are being <a href=\"#{ERB::Util.unwrapped_html_escape(location)}\">redirected</a>.</body></html>"