    # then credentials.secret_key_base, and finally secrets.secret_key_base. For most applications,
    # the correct place to store it is in the encrypted credentials file.
    def secret_key_base
      if Rails.env.development? || Rails.env.test?
        secrets.secret_key_base ||= generate_development_secret
      else
        validate_secret_key_base(
          ENV["SECRET_KEY_BASE"] || credentials.secret_key_base || secrets.secret_key_base
        )

    private

      def generate_development_secret
        if secrets.secret_key_base.nil?
          key_file = Rails.root.join("tmp/development_secret.txt")

          if !File.exist?(key_file)
            random_key = SecureRandom.hex(64)
            File.binwrite(key_file, random_key)
          end

          secrets.secret_key_base = File.binread(key_file)
        end

        secrets.secret_key_base
      end

      def build_request(env)
        req = super
        env["ORIGINAL_FULLPATH"] = req.fullpath
        env["ORIGINAL_SCRIPT_NAME"] = req.script_name