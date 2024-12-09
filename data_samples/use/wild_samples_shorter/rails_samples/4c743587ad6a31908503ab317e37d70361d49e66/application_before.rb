    # then credentials.secret_key_base, and finally secrets.secret_key_base. For most applications,
    # the correct place to store it is in the encrypted credentials file.
    def secret_key_base
      if Rails.env.test? || Rails.env.development?
        secrets.secret_key_base || Digest::MD5.hexdigest(self.class.name)
      else
        validate_secret_key_base(
          ENV["SECRET_KEY_BASE"] || credentials.secret_key_base || secrets.secret_key_base
        )

    private

      def build_request(env)
        req = super
        env["ORIGINAL_FULLPATH"] = req.fullpath
        env["ORIGINAL_SCRIPT_NAME"] = req.script_name