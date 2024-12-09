          request.cookie_jar[key] = cookie
        end
    end

    class AbstractSecureStore < Rack::Session::Abstract::PersistedSecure
      include Compatibility
      include StaleSessionCheck
      include SessionObject

      def generate_sid
        Rack::Session::SessionId.new(super)
      end

      private
        def set_cookie(request, session_id, cookie)
          request.cookie_jar[key] = cookie
        end
    end
  end
end