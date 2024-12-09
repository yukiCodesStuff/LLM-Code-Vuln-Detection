      # Support raw values in the local cache strategy.
      module LocalCacheWithRaw # :nodoc:
        private
          def read_entry(key, **options)
            entry = super
            if options[:raw] && local_cache && entry
              entry = deserialize_entry(entry.value)
            end
            entry
          end

          def write_entry(key, entry, **options)
            if options[:raw] && local_cache
              raw_entry = Entry.new(serialize_entry(entry, raw: true))
              raw_entry.expires_at = entry.expires_at
        # Read an entry from the cache.
        def read_entry(key, **options)
          failsafe :read_entry do
            deserialize_entry redis.with { |c| c.get(key) }
          end
        end

        def read_multi_entries(names, **options)
          options = names.extract_options!
          options = merged_options(options)
          return {} if names == []

          keys = names.map { |name| normalize_key(name, options) }

          values = failsafe(:read_multi_mget, returning: {}) do

          names.zip(values).each_with_object({}) do |(name, value), results|
            if value
              entry = deserialize_entry(value)
              unless entry.nil? || entry.expired? || entry.mismatched?(normalize_version(name, options))
                results[name] = entry.value
              end
            end
          end
        end

        def deserialize_entry(serialized_entry)
          if serialized_entry
            entry = Marshal.load(serialized_entry) rescue serialized_entry
            entry.is_a?(Entry) ? entry : Entry.new(entry)
          end
        end

        def serialize_entry(entry, raw: false)