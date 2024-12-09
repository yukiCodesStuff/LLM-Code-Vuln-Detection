
  SET              = Mimes.new
  EXTENSION_LOOKUP = {}
  LOOKUP           = {}

  class << self
    def [](type)
      return type if type.is_a?(Type)
      end

      def lookup(string)
        LOOKUP[string] || Type.new(string)
      end

      def lookup_by_extension(extension)
        EXTENSION_LOOKUP[extension.to_s]
      end
    end

    attr_reader :hash

    def initialize(string, symbol = nil, synonyms = [])
      @symbol, @synonyms = symbol, synonyms
      @string = string
      @hash = [@string, @synonyms, @symbol].hash
    end

    def to_s
      @string
      end
    end

    def eql?(other)
      super || (self.class == other.class &&
                @string    == other.string &&
                @synonyms  == other.synonyms &&
                @symbol    == other.symbol)
    end

    def =~(mime_type)
      return false unless mime_type
      regexp = Regexp.new(Regexp.quote(mime_type.to_s))
      @synonyms.any? { |synonym| synonym.to_s =~ regexp } || @string =~ regexp

    def all?; false; end

    protected

    attr_reader :string, :synonyms

    private

    def to_ary; end
    def to_a; end