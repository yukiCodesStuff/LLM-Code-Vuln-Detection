class FinderTest < ActiveRecord::TestCase
  fixtures :companies, :topics, :entrants, :developers, :developers_projects, :posts, :comments, :accounts, :authors, :customers, :categories, :categorizations

  def test_find
    assert_equal(topics(:first).title, Topic.find(1).title)
  end
