end

require "active_support/core_ext/enumerable"
require "active_support/core_ext/array/extract_options"

module ActiveSupport
  module Cache
      # Provide support for raw values in the local cache strategy.
      module LocalCacheWithRaw # :nodoc:
        private
          def write_entry(key, entry, **options)
            if options[:raw] && local_cache
              raw_entry = Entry.new(entry.value.to_s)
              raw_entry.expires_at = entry.expires_at
          key
        end

        def deserialize_entry(entry)
          if entry
            entry.is_a?(Entry) ? entry : Entry.new(entry)
          end
        end
