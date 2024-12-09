    assert_equal "#{@content} and went by the lake", @encrypted_file.read
  end

  test "change sets restricted permissions" do
    @encrypted_file.write(@content)
    @encrypted_file.change do |file|
      assert_predicate file, :owned?
      assert_equal "100600", file.stat.mode.to_s(8), "Incorrect mode for #{file}"
    end
  end

  test "raise MissingKeyError when key is missing" do
    assert_raise ActiveSupport::EncryptedFile::MissingKeyError do
      encrypted_file(@content_path, key_path: "", env_key: "").read
    end