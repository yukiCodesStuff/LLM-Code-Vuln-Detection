      sanitize_action_text_content(render_action_text_attachments(content))
    end

    def sanitize_action_text_content(content)
      sanitizer.sanitize(
        content.to_html,
        tags: sanitizer_allowed_tags,