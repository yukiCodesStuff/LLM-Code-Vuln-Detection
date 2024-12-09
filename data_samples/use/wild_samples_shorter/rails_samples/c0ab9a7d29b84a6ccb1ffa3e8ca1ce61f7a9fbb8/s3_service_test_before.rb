      @service.delete key
    end

    test "upload a zero byte file" do
      blob = directly_upload_file_blob filename: "empty_file.txt", content_type: nil
      user = User.create! name: "DHH", avatar: blob
