    MIME_NAME = "[a-zA-Z0-9][a-zA-Z0-9#{Regexp.escape('!#$&-^_.+')}]{0,126}"
    MIME_PARAMETER_VALUE = "#{Regexp.escape('"')}?#{MIME_NAME}#{Regexp.escape('"')}?"
    MIME_PARAMETER = "\s*\;\s*#{MIME_NAME}(?:\=#{MIME_PARAMETER_VALUE})?"
    MIME_REGEXP = /\A(?:\*\/\*|#{MIME_NAME}\/(?:\*|#{MIME_NAME})(?>\s*#{MIME_PARAMETER}\s*)*)\z/

    class InvalidMimeType < StandardError; end

    def initialize(string, symbol = nil, synonyms = [])