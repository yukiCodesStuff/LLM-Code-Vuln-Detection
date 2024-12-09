    # * <tt>cache</tt>         - The cache to use. If it is not specified, <tt>Rails.cache</tt> will be used.
    # * <tt>expire_after</tt>  - The length of time a session will be stored before automatically expiring.
    #   By default, the <tt>:expires_in</tt> option of the cache is used.
    class CacheStore < AbstractStore
      def initialize(app, options = {})
        @cache = options[:cache] || Rails.cache
        options[:expire_after] ||= @cache.options[:expires_in]
        super

      # Get a session from the cache.
      def find_session(env, sid)
        unless sid && (session = @cache.read(cache_key(sid)))
          sid, session = generate_sid, {}
        end
        [sid, session]
      end

      # Set a session in the cache.
      def write_session(env, sid, session, options)
        key = cache_key(sid)
        if session
          @cache.write(key, session, expires_in: options[:expire_after])
        else
          @cache.delete(key)

      # Remove a session from the cache.
      def delete_session(env, sid, options)
        @cache.delete(cache_key(sid))
        generate_sid
      end

      private
        # Turn the session id into a cache key.
        def cache_key(sid)
          "_session_id:#{sid}"
        end
    end
  end
end