      def call(env)
        _, headers, _ = response = @app.call(env)

        return response unless html_response?(headers)
        return response if policy_present?(headers)

        request = ActionDispatch::Request.new(env)

      end

      private
        def html_response?(headers)
          if content_type = headers[Rack::CONTENT_TYPE]
            content_type.include?("html")
          end
        end

        def policy_present?(headers)
          headers[ActionDispatch::Constants::FEATURE_POLICY]
        end
