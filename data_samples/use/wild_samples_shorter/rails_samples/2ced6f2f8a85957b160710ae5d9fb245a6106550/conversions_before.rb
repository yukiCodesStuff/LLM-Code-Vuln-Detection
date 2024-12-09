    #
    #   hash = Hash.from_xml(xml)
    #   # => {"hash"=>{"foo"=>1, "bar"=>2}}
    def from_xml(xml)
      ActiveSupport::XMLConverter.new(xml).to_h
    end

  end
end

module ActiveSupport
  class XMLConverter # :nodoc:
    def initialize(xml)
      @xml = normalize_keys(XmlMini.parse(xml))
    end

    def to_h
      deep_to_h(@xml)
    end

    private

      def normalize_keys(params)
        case params
          when Hash
            Hash[params.map { |k,v| [k.to_s.tr('-', '_'), normalize_keys(v)] } ]
      end

      def process_hash(value)
        if become_array?(value)
          _, entries = Array.wrap(value.detect { |k,v| not v.is_a?(String) })
          if entries.nil? || value['__content__'].try(:empty?)
            []