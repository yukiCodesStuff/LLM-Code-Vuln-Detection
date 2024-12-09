    end

    protected
      # The actual before_filter that is used.  Modify this to change how you handle unverified requests.
      def verify_authenticity_token
        verified_request? || handle_unverified_request
      end

      def handle_unverified_request
        reset_session
      end

      # Returns true or false if a request is verified.  Checks:
      #
      # * is it a GET request?  Gets should be safe and idempotent
      # * Does the form_authenticity_token match the given token value from the params?
      # * Does the X-CSRF-Token header match the form_authenticity_token
      def verified_request?
        !protect_against_forgery? || request.get? ||
          form_authenticity_token == params[request_forgery_protection_token] ||
          form_authenticity_token == request.headers['X-CSRF-Token']
      end

      # Sets the token value for the current session.
      def form_authenticity_token