# frozen_string_literal: true

require "erb"
require "action_dispatch/http/request"
require "active_support/actionable_error"

module ActionDispatch
      end

      def redirect_to(location)
        body = "<html><body>You are being <a href=\"#{ERB::Util.unwrapped_html_escape(location)}\">redirected</a>.</body></html>"

        [302, {
          "Content-Type" => "text/html; charset=#{Response.default_charset}",
          "Content-Length" => body.bytesize.to_s,