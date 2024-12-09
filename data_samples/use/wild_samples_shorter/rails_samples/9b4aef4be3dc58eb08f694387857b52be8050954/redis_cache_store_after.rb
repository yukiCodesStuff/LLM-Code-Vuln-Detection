      # Support raw values in the local cache strategy.
      module LocalCacheWithRaw # :nodoc:
        private
          def write_entry(key, entry, **options)
            if options[:raw] && local_cache
              raw_entry = Entry.new(serialize_entry(entry, raw: true))
              raw_entry.expires_at = entry.expires_at
        # Read an entry from the cache.
        def read_entry(key, **options)
          failsafe :read_entry do
            raw = options&.fetch(:raw, false)
            deserialize_entry(redis.with { |c| c.get(key) }, raw: raw)
          end
        end

        def read_multi_entries(names, **options)
          options = names.extract_options!
          options = merged_options(options)
          return {} if names == []
          raw = options&.fetch(:raw, false)

          keys = names.map { |name| normalize_key(name, options) }

          values = failsafe(:read_multi_mget, returning: {}) do

          names.zip(values).each_with_object({}) do |(name, value), results|
            if value
              entry = deserialize_entry(value, raw: raw)
              unless entry.nil? || entry.expired? || entry.mismatched?(normalize_version(name, options))
                results[name] = entry.value
              end
            end
          end
        end

        def deserialize_entry(serialized_entry, raw:)
          if serialized_entry
            if raw
              Entry.new(serialized_entry)
            else
              Marshal.load(serialized_entry)
            end
          end
        end

        def serialize_entry(entry, raw: false)