          action_path = normalize_action_path(action)
          per_form_csrf_token(session, action_path, method)
        else
          real_csrf_token(session)
        end

        one_time_pad = SecureRandom.random_bytes(AUTHENTICITY_TOKEN_LENGTH)
        encrypted_csrf_token = xor_byte_strings(one_time_pad, raw_token)
        masked_token = one_time_pad + encrypted_csrf_token
        Base64.urlsafe_encode64(masked_token, padding: false)
      end

      # Checks the client's masked token to see if it matches the
      # session token. Essentially the inverse of
        elsif masked_token.length == AUTHENTICITY_TOKEN_LENGTH * 2
          csrf_token = unmask_token(masked_token)

          compare_with_real_token(csrf_token, session) ||
            valid_per_form_csrf_token?(csrf_token, session)
        else
          false # Token is malformed.
        end
        xor_byte_strings(one_time_pad, encrypted_csrf_token)
      end

      def compare_with_real_token(token, session) # :doc:
        ActiveSupport::SecurityUtils.fixed_length_secure_compare(token, real_csrf_token(session))
      end

      def valid_per_form_csrf_token?(token, session) # :doc:
        if per_form_csrf_tokens
          correct_token = per_form_csrf_token(
            session,
      end

      def per_form_csrf_token(session, action_path, method) # :doc:
        OpenSSL::HMAC.digest(
          OpenSSL::Digest::SHA256.new,
          real_csrf_token(session),
          [action_path, method.downcase].join("#")
        )
      end

      def xor_byte_strings(s1, s2) # :doc: