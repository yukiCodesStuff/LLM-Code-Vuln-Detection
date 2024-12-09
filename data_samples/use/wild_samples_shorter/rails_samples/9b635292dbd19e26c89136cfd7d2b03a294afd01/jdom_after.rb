        xml_string_reader = StringReader.new(data)
        xml_input_source = InputSource.new(xml_string_reader)
        doc = @dbf.new_document_builder.parse(xml_input_source)
        merge_element!({CONTENT_KEY => ''}, doc.document_element, XmlMini.depth)
      end
    end

    private
    #   Hash to merge the converted element into.
    # element::
    #   XML element to merge into hash
    def merge_element!(hash, element, depth)
      raise 'Document too deep!' if depth == 0
      delete_empty(hash)
      merge!(hash, element.tag_name, collapse(element, depth))
    end

    def delete_empty(hash)
      hash.delete(CONTENT_KEY) if hash[CONTENT_KEY] == ''
    #
    # element::
    #   The document element to be collapsed.
    def collapse(element, depth)
      hash = get_attributes(element)

      child_nodes = element.child_nodes
      if child_nodes.length > 0
        (0...child_nodes.length).each do |i|
          child = child_nodes.item(i)
          merge_element!(hash, child, depth - 1) unless child.node_type == Node.TEXT_NODE
        end
        merge_texts!(hash, element) unless empty_content?(element)
        hash
      else