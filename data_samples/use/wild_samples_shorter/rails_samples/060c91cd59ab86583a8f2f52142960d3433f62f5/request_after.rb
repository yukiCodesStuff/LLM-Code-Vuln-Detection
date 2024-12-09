      LOCALHOST =~ remote_addr && LOCALHOST =~ remote_ip
    end

    protected

    # Remove nils from the params hash
    def deep_munge(hash)
      hash.each_value do |v|
        case v
        when Array
          v.grep(Hash) { |x| deep_munge(x) }
        when Hash
          deep_munge(v)
        end
      end

      keys = hash.keys.find_all { |k| hash[k] == [nil] }
      keys.each { |k| hash[k] = nil }
      hash
    end

    def parse_query(qs)
      deep_munge(super)
    end

    private

    def check_method(name)
      HTTP_METHOD_LOOKUP[name] || raise(ActionController::UnknownHttpMethod, "#{name}, accepted HTTP methods are #{HTTP_METHODS.to_sentence(:locale => :en)}")