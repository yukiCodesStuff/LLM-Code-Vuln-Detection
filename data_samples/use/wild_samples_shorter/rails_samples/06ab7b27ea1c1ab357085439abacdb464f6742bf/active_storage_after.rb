  mattr_accessor :paths, default: {}
  mattr_accessor :variable_content_types, default: []
  mattr_accessor :content_types_to_serve_as_binary, default: []
  mattr_accessor :content_types_allowed_inline, default: []
  mattr_accessor :binary_content_type, default: "application/octet-stream"
  mattr_accessor :service_urls_expire_in, default: 5.minutes
  mattr_accessor :routes_prefix, default: "/rails/active_storage"

  module Transformers