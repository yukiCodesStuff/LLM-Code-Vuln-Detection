require "cases/helper"
require "models/admin"
require "models/admin/user"
require "models/account"

class StoreTest < ActiveRecord::TestCase
  fixtures :'admin/users'
      parent_name: "Quinn", partner_name: "Dallas",
      partner_birthday: "1997-11-1"
    )
  end

  test "reading store attributes through accessors" do
    assert_equal "black", @john.color
    assert_equal "heavy", @john.json_data["weight"]
  end

  test "convert store attributes from Hash to HashWithIndifferentAccess saving the data and access attributes indifferently" do
    user = Admin::User.find_by_name("Jamis")
    assert_equal "symbol",  user.settings[:symbol]
    assert_equal "symbol",  user.settings["symbol"]
    assert_equal "string",  user.settings[:string]
    assert_equal [:secret_question, :two_factor_auth, :login_retry], Admin::User.stored_attributes[:configs]
  end
end