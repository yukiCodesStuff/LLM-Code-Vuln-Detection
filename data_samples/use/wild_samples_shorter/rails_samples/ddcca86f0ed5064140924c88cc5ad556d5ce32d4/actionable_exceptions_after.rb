# frozen_string_literal: true

require "erb"
require "uri"
require "action_dispatch/http/request"
require "active_support/actionable_error"

module ActionDispatch
      end

      def redirect_to(location)
        uri = URI.parse location

        if uri.relative? || uri.scheme == "http" || uri.scheme == "https"
          body = "<html><body>You are being <a href=\"#{ERB::Util.unwrapped_html_escape(location)}\">redirected</a>.</body></html>"
        else
          return [400, {"Content-Type" => "text/plain"}, ["Invalid redirection URI"]]
        end

        [302, {
          "Content-Type" => "text/html; charset=#{Response.default_charset}",
          "Content-Length" => body.bytesize.to_s,