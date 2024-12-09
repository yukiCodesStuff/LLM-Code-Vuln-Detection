      end

      def decode_credentials(header)
        Hash[header.to_s.gsub(/^Digest\s+/,'').split(',').map do |pair|
          key, value = pair.split('=', 2)
          [key.strip.to_sym, value.to_s.gsub(/^"|"$/,'').delete('\'')]
        end]
      end

      def authentication_header(controller, realm)