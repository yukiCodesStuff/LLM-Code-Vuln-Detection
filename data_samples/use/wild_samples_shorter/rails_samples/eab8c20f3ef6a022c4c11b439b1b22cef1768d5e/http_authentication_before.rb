    module Token
      TOKEN_KEY = "token="
      TOKEN_REGEX = /^(Token|Bearer)\s+/
      AUTHN_PAIR_DELIMITERS = /(?:,|;|\t+)/
      extend self

      module ControllerMethods
        def authenticate_or_request_with_http_token(realm = "Application", message = nil, &login_procedure)