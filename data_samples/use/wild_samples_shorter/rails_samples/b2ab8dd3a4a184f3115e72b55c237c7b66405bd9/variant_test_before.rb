    assert_equal :png, blob.send(:default_variant_format)
  end

  private
    def process_variants_with(processor)
      previous_processor, ActiveStorage.variant_processor = ActiveStorage.variant_processor, processor
      yield