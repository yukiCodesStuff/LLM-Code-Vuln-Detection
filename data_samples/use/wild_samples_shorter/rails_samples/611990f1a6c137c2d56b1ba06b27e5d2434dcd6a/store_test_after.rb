require "cases/helper"
require "models/admin"
require "models/admin/user"
require "models/admin/user_json"
require "models/account"

class StoreTest < ActiveRecord::TestCase
  fixtures :'admin/users'
      parent_name: "Quinn", partner_name: "Dallas",
      partner_birthday: "1997-11-1"
    )
    ActiveRecord.use_yaml_unsafe_load = true
  end

  test "reading store attributes through accessors" do
    assert_equal "black", @john.color
    assert_equal "heavy", @john.json_data["weight"]
  end

  def test_convert_store_attributes_from_Hash_to_HashWithIndifferentAccess_saving_the_data_and_access_attributes_indifferently
    user = Admin::User.find_by_name("Jamis")
    assert_equal "symbol",  user.settings[:symbol]
    assert_equal "symbol",  user.settings["symbol"]
    assert_equal "string",  user.settings[:string]
    assert_equal [:secret_question, :two_factor_auth, :login_retry], Admin::User.stored_attributes[:configs]
  end
end

class StoreTestWithYAMLSafeLoad < StoreTest
  fixtures :'admin/users'

  setup do
    @john = Admin::UserJSON.create!(
      name: "Jim Doe", color: "black", remember_login: true,
      height: "tall", is_a_good_guy: true,
      parent_name: "Quinn", partner_name: "Dallas",
      partner_birthday: "1997-11-1"
    )
    ActiveRecord.use_yaml_unsafe_load = false
  end

  def test_convert_store_attributes_from_Hash_to_HashWithIndifferentAccess_saving_the_data_and_access_attributes_indifferently
    skip "Symbol is not a supported class in Psych::safe_load"
  end
end