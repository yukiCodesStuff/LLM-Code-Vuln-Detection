
      class << self
        # Remove nils from the params hash
        def deep_munge(hash)
          return hash unless perform_deep_munge

          hash.each do |k, v|
            case v
            when Array
              v.grep(Hash) { |x| deep_munge(x) }
              v.compact!
              hash[k] = nil if v.empty?
            when Hash
              deep_munge(v)
            end
          end

          hash
        end