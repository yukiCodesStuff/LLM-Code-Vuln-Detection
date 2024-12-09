    #
    #   hash = Hash.from_xml(xml)
    #   # => {"hash"=>{"foo"=>1, "bar"=>2}}
    #
    # DisallowedType is raise if the XML contains attributes with <tt>type="yaml"</tt> or
    # <tt>type="symbol"</tt>. Use <tt>Hash.from_trusted_xml</tt> to parse this XML.
    def from_xml(xml, disallowed_types = nil)
      ActiveSupport::XMLConverter.new(xml, disallowed_types).to_h
    end

    # Builds a Hash from XML just like <tt>Hash.from_xml</tt>, but also allows Symbol and YAML.
    def from_trusted_xml(xml)
      from_xml xml, []
    end
  end
end

module ActiveSupport
  class XMLConverter # :nodoc:
    class DisallowedType < StandardError
      def initialize(type)
        super "Disallowed type attribute: #{type.inspect}"
      end
    end

    DISALLOWED_TYPES = %w(symbol yaml)

    def initialize(xml, disallowed_types = nil)
      @xml = normalize_keys(XmlMini.parse(xml))
      @disallowed_types = disallowed_types || DISALLOWED_TYPES
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
        if value.include?('type') && !value['type'].is_a?(Hash) && @disallowed_types.include?(value['type'])
          raise DisallowedType, value['type']
        end

        if become_array?(value)
          _, entries = Array.wrap(value.detect { |k,v| not v.is_a?(String) })
          if entries.nil? || value['__content__'].try(:empty?)
            []