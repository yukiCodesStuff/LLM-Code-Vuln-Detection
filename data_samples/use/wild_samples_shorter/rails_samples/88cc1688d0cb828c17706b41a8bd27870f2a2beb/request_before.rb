      LOCALHOST =~ remote_addr && LOCALHOST =~ remote_ip
    end

    protected

    # Remove nils from the params hash
    def deep_munge(hash)
      hash.each_value do |v|
        case v
        when Array
          v.grep(Hash) { |x| deep_munge(x) }
          v.compact!
        when Hash
          deep_munge(v)
        end
      end
      hash
    end

    def parse_query(qs)
      deep_munge(super)
    end
