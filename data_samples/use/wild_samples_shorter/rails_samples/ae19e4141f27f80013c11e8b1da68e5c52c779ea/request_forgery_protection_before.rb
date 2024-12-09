    end

    protected

      def protect_from_forgery(options = {})
        self.request_forgery_protection_token ||= :authenticity_token
        before_filter :verify_authenticity_token, options
      end

      # The actual before_filter that is used.  Modify this to change how you handle unverified requests.
      def verify_authenticity_token
        verified_request? || raise(ActionController::InvalidAuthenticityToken)
      end

      # Returns true or false if a request is verified.  Checks:
      #
      # * is the format restricted?  By default, only HTML requests are checked.
      # * is it a GET request?  Gets should be safe and idempotent
      # * Does the form_authenticity_token match the given token value from the params?
      def verified_request?
        !protect_against_forgery? || request.forgery_whitelisted? ||
          form_authenticity_token == params[request_forgery_protection_token]
      end

      # Sets the token value for the current session.
      def form_authenticity_token