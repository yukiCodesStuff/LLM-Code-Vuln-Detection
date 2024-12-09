  mattr_accessor :content_types_to_serve_as_binary, default: []
  mattr_accessor :content_types_allowed_inline,     default: []

  mattr_accessor :service_urls_expire_in, default: 5.minutes
  mattr_accessor :urls_expire_in

  mattr_accessor :routes_prefix, default: "/rails/active_storage"