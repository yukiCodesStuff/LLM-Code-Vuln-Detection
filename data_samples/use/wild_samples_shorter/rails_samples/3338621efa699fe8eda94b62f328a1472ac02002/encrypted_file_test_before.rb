    assert_equal "#{@content} and went by the lake", @encrypted_file.read
  end

  test "raise MissingKeyError when key is missing" do
    assert_raise ActiveSupport::EncryptedFile::MissingKeyError do
      encrypted_file(@content_path, key_path: "", env_key: "").read
    end