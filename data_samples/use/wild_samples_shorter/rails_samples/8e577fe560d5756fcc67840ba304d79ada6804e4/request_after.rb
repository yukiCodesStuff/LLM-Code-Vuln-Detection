      LOCALHOST =~ remote_addr && LOCALHOST =~ remote_ip
    end

    # Remove nils from the params hash
    def deep_munge(hash)
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

    protected

    def parse_query(qs)
      deep_munge(super)
    end
