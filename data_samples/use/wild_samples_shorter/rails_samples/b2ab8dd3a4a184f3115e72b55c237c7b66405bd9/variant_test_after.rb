    assert_equal :png, blob.send(:default_variant_format)
  end

  test "variations with dangerous argument string raise UnsupportedImageProcessingArgument" do
    process_variants_with :mini_magick do
      blob = create_file_blob(filename: "racecar.jpg")
      assert_raise(ActiveStorage::Transformers::ImageProcessingTransformer::UnsupportedImageProcessingArgument) do
        blob.variant(resize: "-PaTh /tmp/file.erb").processed
      end
    end
  end

  test "variations with dangerous argument array raise UnsupportedImageProcessingArgument" do
    process_variants_with :mini_magick do
      blob = create_file_blob(filename: "racecar.jpg")
      assert_raise(ActiveStorage::Transformers::ImageProcessingTransformer::UnsupportedImageProcessingArgument) do
        blob.variant(resize: [123, "-write", "/tmp/file.erb"]).processed
      end
    end
  end

  test "variations with dangerous argument in a nested array raise UnsupportedImageProcessingArgument" do
    process_variants_with :mini_magick do
      blob = create_file_blob(filename: "racecar.jpg")
      assert_raise(ActiveStorage::Transformers::ImageProcessingTransformer::UnsupportedImageProcessingArgument) do
        blob.variant(resize: [123, ["-write", "/tmp/file.erb"], "/tmp/file.erb"]).processed
      end

      assert_raise(ActiveStorage::Transformers::ImageProcessingTransformer::UnsupportedImageProcessingArgument) do
        blob.variant(resize: [123, {"-write /tmp/file.erb": "something"}, "/tmp/file.erb"]).processed
      end
    end
  end

  test "variations with dangerous argument hash raise UnsupportedImageProcessingArgument" do
    process_variants_with :mini_magick do
      blob = create_file_blob(filename: "racecar.jpg")
      assert_raise(ActiveStorage::Transformers::ImageProcessingTransformer::UnsupportedImageProcessingArgument) do
        blob.variant(saver: {"-write": "/tmp/file.erb"}).processed
      end
    end
  end

  test "variations with dangerous argument in a nested hash raise UnsupportedImageProcessingArgument" do
    process_variants_with :mini_magick do
      blob = create_file_blob(filename: "racecar.jpg")
      assert_raise(ActiveStorage::Transformers::ImageProcessingTransformer::UnsupportedImageProcessingArgument) do
        blob.variant(saver: {"something": {"-write": "/tmp/file.erb"}}).processed
      end

      assert_raise(ActiveStorage::Transformers::ImageProcessingTransformer::UnsupportedImageProcessingArgument) do
        blob.variant(saver: {"something": ["-write", "/tmp/file.erb"]}).processed
      end
    end
  end

  test "variations with unsupported methods raise UnsupportedImageProcessingMethod" do
    process_variants_with :mini_magick do
      blob = create_file_blob(filename: "racecar.jpg")
      assert_raise(ActiveStorage::Transformers::ImageProcessingTransformer::UnsupportedImageProcessingMethod) do
        blob.variant(system: "touch /tmp/dangerous").processed
      end
    end
  end

  private
    def process_variants_with(processor)
      previous_processor, ActiveStorage.variant_processor = ActiveStorage.variant_processor, processor
      yield