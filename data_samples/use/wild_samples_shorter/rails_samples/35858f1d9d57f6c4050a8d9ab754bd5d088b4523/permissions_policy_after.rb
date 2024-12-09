      def call(env)
        _, headers, _ = response = @app.call(env)

        return response if policy_present?(headers)

        request = ActionDispatch::Request.new(env)

      end

      private
        def policy_present?(headers)
          headers[ActionDispatch::Constants::FEATURE_POLICY]
        end
