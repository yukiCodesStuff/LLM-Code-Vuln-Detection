      @service.delete key
    end

    test "directly uploading file larger than the provided content-length does not work" do
      key      = SecureRandom.base58(24)
      data     = "Some text that is longer than the specified content length"
      checksum = Digest::MD5.base64digest(data)
      url      = @service.url_for_direct_upload(key, expires_in: 5.minutes, content_type: "text/plain", content_length: data.size - 1, checksum: checksum)

      uri = URI.parse url
      request = Net::HTTP::Put.new uri.request_uri
      request.body = data
      request.add_field "Content-Type", "text/plain"
      request.add_field "Content-MD5", checksum
      upload_result = Net::HTTP.start(uri.host, uri.port, use_ssl: true) do |http|
        http.request request
      end

      assert_equal "403", upload_result.code
      assert_raises ActiveStorage::FileNotFoundError do
        @service.download(key)
      end
    ensure
      @service.delete key
    end

    test "upload a zero byte file" do
      blob = directly_upload_file_blob filename: "empty_file.txt", content_type: nil
      user = User.create! name: "DHH", avatar: blob
