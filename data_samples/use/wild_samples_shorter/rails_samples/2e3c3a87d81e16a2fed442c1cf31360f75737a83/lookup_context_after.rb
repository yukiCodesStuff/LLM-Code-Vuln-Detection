      @details_keys = ThreadSafe::Cache.new

      def self.get(details)
        if details[:formats]
          details = details.dup
          syms    = Set.new Mime::SET.symbols
          details[:formats] = details[:formats].select { |v|
            syms.include? v
          }
        end
        @details_keys[details] ||= new
      end

      def self.clear