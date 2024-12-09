    end
  end

  test "urls force content_type to binary and attachment as content disposition for content types served as binary" do
    blob = create_blob(content_type: "text/html")

    freeze_time do
      assert_equal expected_url_for(blob, disposition: :attachment, content_type: "application/octet-stream"), blob.service_url
      assert_equal expected_url_for(blob, disposition: :attachment, content_type: "application/octet-stream"), blob.service_url(disposition: :inline)
    end
  end

  test "urls force attachment as content disposition when the content type is not allowed inline" do
    blob = create_blob(content_type: "application/zip")

    freeze_time do
      assert_equal expected_url_for(blob, disposition: :attachment, content_type: "application/zip"), blob.service_url
      assert_equal expected_url_for(blob, disposition: :attachment, content_type: "application/zip"), blob.service_url(disposition: :inline)
    end
  end

  test "urls allow for custom filename" do
    arguments = [
      blob.key,
      expires_in: ActiveStorage.service_urls_expire_in,
      disposition: :attachment,
      content_type: blob.content_type,
      filename: blob.filename,
      thumb_size: "300x300",
      thumb_mode: "crop"
  end

  private
    def expected_url_for(blob, disposition: :attachment, filename: nil, content_type: nil)
      filename ||= blob.filename
      content_type ||= blob.content_type

      query = { disposition: disposition.to_s + "; #{filename.parameters}", content_type: content_type }
      key_params = { key: blob.key }.merge(query)

      "https://example.com/rails/active_storage/disk/#{ActiveStorage.verifier.generate(key_params, expires_in: 5.minutes, purpose: :blob_key)}/#{filename}?#{query.to_param}"
    end
end