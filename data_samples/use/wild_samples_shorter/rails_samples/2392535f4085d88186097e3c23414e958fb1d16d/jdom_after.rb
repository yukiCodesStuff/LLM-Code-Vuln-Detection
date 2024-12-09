        {}
      else
        @dbf = DocumentBuilderFactory.new_instance
        # secure processing of java xml
        # http://www.ibm.com/developerworks/xml/library/x-tipcfsx/index.html
        @dbf.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false)
        @dbf.setFeature("http://xml.org/sax/features/external-general-entities", false)
        @dbf.setFeature("http://xml.org/sax/features/external-parameter-entities", false)
        @dbf.setFeature(javax.xml.XMLConstants::FEATURE_SECURE_PROCESSING, true)
        xml_string_reader = StringReader.new(data)
        xml_input_source = InputSource.new(xml_string_reader)
        doc = @dbf.new_document_builder.parse(xml_input_source)
        merge_element!({CONTENT_KEY => ''}, doc.document_element)