
  SET              = Mimes.new
  EXTENSION_LOOKUP = {}
  LOOKUP           = Hash.new { |h, k| h[k] = Type.new(k) unless k.blank? }

  class << self
    def [](type)
      return type if type.is_a?(Type)
      end

      def lookup(string)
        LOOKUP[string]
      end

      def lookup_by_extension(extension)
        EXTENSION_LOOKUP[extension.to_s]
      end
    end

    def initialize(string, symbol = nil, synonyms = [])
      @symbol, @synonyms = symbol, synonyms
      @string = string
    end

    def to_s
      @string
      end
    end

    def =~(mime_type)
      return false unless mime_type
      regexp = Regexp.new(Regexp.quote(mime_type.to_s))
      @synonyms.any? { |synonym| synonym.to_s =~ regexp } || @string =~ regexp

    def all?; false; end

    private

    def to_ary; end
    def to_a; end