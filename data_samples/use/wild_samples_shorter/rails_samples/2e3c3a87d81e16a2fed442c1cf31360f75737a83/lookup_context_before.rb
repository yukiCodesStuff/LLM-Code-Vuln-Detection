      @details_keys = ThreadSafe::Cache.new

      def self.get(details)
        @details_keys[details] ||= new
      end

      def self.clear