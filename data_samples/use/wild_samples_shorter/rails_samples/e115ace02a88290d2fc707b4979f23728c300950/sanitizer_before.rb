
    # A regular expression of the valid characters used to separate protocols like
    # the ':' in 'http://foo.com'
    self.protocol_separator     = /:|(&#0*58)|(&#x70)|(%|&#37;)3A/

    # Specifies a Set of HTML attributes that can have URIs.
    self.uri_attributes         = Set.new(%w(href src cite action longdesc xlink:href lowsrc))


    def contains_bad_protocols?(attr_name, value)
      uri_attributes.include?(attr_name) &&
      (value =~ /(^[^\/:]*):|(&#0*58)|(&#x70)|(%|&#37;)3A/ && !allowed_protocols.include?(value.split(protocol_separator).first.downcase.strip))
    end
  end
end