  #     body.to_s # => "<h1>Funny times!</h1>"
  #     body.to_plain_text # => "Funny times!"
  class Content
    include Rendering, Serialization

    attr_reader :fragment

    delegate :deconstruct, to: :fragment

    def render_attachments(**options, &block)
      content = fragment.replace(ActionText::Attachment.tag_name) do |node|
        block.call(attachment_for_node(node, **options))
      end
      self.class.new(content, canonicalize: false)
    end