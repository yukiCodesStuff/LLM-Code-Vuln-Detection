require 'digest'

module ActiveSupport
  module SecurityUtils
    # Constant time string comparison.
    #
      res == 0
    end
    module_function :secure_compare

    def variable_size_secure_compare(a, b) # :nodoc:
      secure_compare(::Digest::SHA256.hexdigest(a), ::Digest::SHA256.hexdigest(b))
    end
    module_function :variable_size_secure_compare
  end
end