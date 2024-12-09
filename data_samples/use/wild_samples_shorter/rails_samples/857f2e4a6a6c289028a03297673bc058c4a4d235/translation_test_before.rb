        end
      end

      def test_localize
        time, expected = Time.gm(2000), "Sat, 01 Jan 2000 00:00:00 +0000"
        I18n.stub :localize, expected do
          assert_equal expected, @controller.l(time)
          assert_equal true, translation.html_safe?
        end
      end
    end
  end
end