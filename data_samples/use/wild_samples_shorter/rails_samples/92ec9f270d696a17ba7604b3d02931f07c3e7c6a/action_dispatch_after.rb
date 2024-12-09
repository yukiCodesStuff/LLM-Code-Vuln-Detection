  end

  module Session
    autoload :AbstractStore,       "action_dispatch/middleware/session/abstract_store"
    autoload :AbstractSecureStore, "action_dispatch/middleware/session/abstract_store"
    autoload :CookieStore,         "action_dispatch/middleware/session/cookie_store"
    autoload :MemCacheStore,       "action_dispatch/middleware/session/mem_cache_store"
    autoload :CacheStore,          "action_dispatch/middleware/session/cache_store"
  end

  mattr_accessor :test_app
