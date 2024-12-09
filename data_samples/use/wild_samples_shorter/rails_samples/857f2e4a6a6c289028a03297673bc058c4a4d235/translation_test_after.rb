        end
      end

      def test_default_translation_as_safe_html
        @controller.stub :action_name, :index do
          translation = @controller.t(".twoz", default: ["<tag>"])
          assert_equal "&lt;tag&gt;", translation
          assert_equal true, translation.html_safe?
        end
      end

      def test_default_translation_with_raise_as_safe_html
        @controller.stub :action_name, :index do
          translation = @controller.t(".twoz", raise: true, default: ["<tag>"])
          assert_equal "&lt;tag&gt;", translation
          assert_equal true, translation.html_safe?
        end
      end

      def test_localize
        time, expected = Time.gm(2000), "Sat, 01 Jan 2000 00:00:00 +0000"
        I18n.stub :localize, expected do
          assert_equal expected, @controller.l(time)
          assert_equal true, translation.html_safe?
        end
      end

      def test_translate_marks_translation_with_missing_html_key_as_safe_html
        @controller.stub :action_name, :index do
          translation = @controller.t("<tag>.html")
          assert_equal "translation missing: <tag>.html", translation
          assert_equal false, translation.html_safe?
        end
      end
      def test_translate_marks_translation_with_missing_nested_html_key_as_safe_html
        @controller.stub :action_name, :index do
          translation = @controller.t(".<tag>.html")
          assert_equal "translation missing: abstract_controller.testing.translation.index.<tag>.html", translation
          assert_equal false, translation.html_safe?
        end
      end
    end
  end
end