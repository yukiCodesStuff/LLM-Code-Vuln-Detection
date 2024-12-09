    end
  end

  test "sanitizes HTML content attachment" do
    attachment = attachment_from_html('<action-text-attachment content-type="text/html" content="<img src=\&quot;.\&quot; onerror=alert>"></action-text-attachment>')
    attachable = attachment.attachable

    ActionText::Content.with_renderer MessagesController.renderer do
      assert_equal "<img src=\"\\%22.\\%22\">", attachable.to_html.strip
    end
  end

  test "defaults trix partial to model partial" do
    attachable = Page.create! title: "Homepage"
    assert_equal "pages/page", attachable.to_trix_content_attachment_partial_path
  end