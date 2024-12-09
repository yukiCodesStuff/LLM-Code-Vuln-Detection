      # +nil+ if the given key is not found in the session.
      def [](key)
        load_for_read!
        @delegate[key.to_s]
      end

      # Returns the nested value specified by the sequence of keys, returning
      # +nil+ if any intermediate step is +nil+.