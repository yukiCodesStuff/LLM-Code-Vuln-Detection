        {}
      else
        @dbf = DocumentBuilderFactory.new_instance
        xml_string_reader = StringReader.new(data)
        xml_input_source = InputSource.new(xml_string_reader)
        doc = @dbf.new_document_builder.parse(xml_input_source)
        merge_element!({CONTENT_KEY => ''}, doc.document_element)