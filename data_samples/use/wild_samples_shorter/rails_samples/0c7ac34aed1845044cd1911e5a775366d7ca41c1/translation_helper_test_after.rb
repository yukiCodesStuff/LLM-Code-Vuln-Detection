  end

  def test_delegates_to_i18n_setting_the_rescue_format_option_to_html
    I18n.expects(:translate).with(:foo, :locale => 'en', :raise=>true).returns("")
    translate :foo, :locale => 'en'
  end

  def test_delegates_localize_to_i18n