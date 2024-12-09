require 'base64'

module ActionController
  # Makes it dead easy to do HTTP Basic, Digest and Token authentication.
  module HttpAuthentication
          def http_basic_authenticate_with(options = {})
            before_action(options.except(:name, :password, :realm)) do
              authenticate_or_request_with_http_basic(options[:realm] || "Application") do |name, password|
                name == options[:name] && password == options[:password]
              end
            end
          end
        end