          content_tag(wrapper_tag, nil, html_options)
        else
          paragraphs.map! { |paragraph|
            content_tag(wrapper_tag, paragraph, html_options, false)
          }.join("\n\n").html_safe
        end
      end
