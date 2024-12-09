    end
  end

  test "urls force attachment as content disposition for content types served as binary" do
    blob = create_blob(content_type: "text/html")

    freeze_time do
      assert_equal expected_url_for(blob, disposition: :attachment), blob.service_url
      assert_equal expected_url_for(blob, disposition: :attachment), blob.service_url(disposition: :inline)
    end
  end

  test "urls allow for custom filename" do
    arguments = [
      blob.key,
      expires_in: ActiveStorage.service_urls_expire_in,
      disposition: :inline,
      content_type: blob.content_type,
      filename: blob.filename,
      thumb_size: "300x300",
      thumb_mode: "crop"
  end

  private
    def expected_url_for(blob, disposition: :inline, filename: nil)
      filename ||= blob.filename
      query_string = { content_type: blob.content_type, disposition: ActionDispatch::Http::ContentDisposition.format(disposition: disposition, filename: filename.sanitized) }.to_param
      "https://example.com/rails/active_storage/disk/#{ActiveStorage.verifier.generate(blob.key, expires_in: 5.minutes, purpose: :blob_key)}/#{filename}?#{query_string}"
    end
end