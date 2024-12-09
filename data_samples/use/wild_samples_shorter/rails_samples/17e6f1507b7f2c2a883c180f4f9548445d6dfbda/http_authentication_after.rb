require 'base64'
require 'active_support/security_utils'

module ActionController
  # Makes it dead easy to do HTTP Basic, Digest and Token authentication.
  module HttpAuthentication
          def http_basic_authenticate_with(options = {})
            before_action(options.except(:name, :password, :realm)) do
              authenticate_or_request_with_http_basic(options[:realm] || "Application") do |name, password|
                # This comparison uses & so that it doesn't short circuit and
                # uses `variable_size_secure_compare` so that length information
                # isn't leaked.
                ActiveSupport::SecurityUtils.variable_size_secure_compare(name, options[:name]) &
                  ActiveSupport::SecurityUtils.variable_size_secure_compare(password, options[:password])
              end
            end
          end
        end