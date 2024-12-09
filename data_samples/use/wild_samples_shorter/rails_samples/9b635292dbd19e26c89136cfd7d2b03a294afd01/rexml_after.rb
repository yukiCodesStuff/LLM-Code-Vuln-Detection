        doc = REXML::Document.new(data)

        if doc.root
          merge_element!({}, doc.root, XmlMini.depth)
        else
          raise REXML::ParseException,
            "The document #{doc.to_s.inspect} does not have a valid root"
        end
      #   Hash to merge the converted element into.
      # element::
      #   XML element to merge into hash
      def merge_element!(hash, element, depth)
        raise REXML::ParseException, "The document is too deep" if depth == 0
        merge!(hash, element.name, collapse(element, depth))
      end

      # Actually converts an XML document element into a data structure.
      #
      # element::
      #   The document element to be collapsed.
      def collapse(element, depth)
        hash = get_attributes(element)

        if element.has_elements?
          element.each_element {|child| merge_element!(hash, child, depth - 1) }
          merge_texts!(hash, element) unless empty_content?(element)
          hash
        else
          merge_texts!(hash, element)