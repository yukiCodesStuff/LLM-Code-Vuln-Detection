    end
  end

  test "defaults trix partial to model partial" do
    attachable = Page.create! title: "Homepage"
    assert_equal "pages/page", attachable.to_trix_content_attachment_partial_path
  end