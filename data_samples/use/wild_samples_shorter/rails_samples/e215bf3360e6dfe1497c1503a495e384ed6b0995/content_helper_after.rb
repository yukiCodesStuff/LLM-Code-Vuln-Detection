      sanitize_action_text_content(render_action_text_attachments(content))
    end

    def sanitize_content_attachment(content_attachment)
      sanitizer.sanitize(
        content_attachment,
        tags: sanitizer_allowed_tags,
        attributes: sanitizer_allowed_attributes,
        scrubber: scrubber,
      )
    end

    def sanitize_action_text_content(content)
      sanitizer.sanitize(
        content.to_html,
        tags: sanitizer_allowed_tags,