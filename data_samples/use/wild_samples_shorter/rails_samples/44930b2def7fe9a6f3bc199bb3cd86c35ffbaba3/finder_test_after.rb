class FinderTest < ActiveRecord::TestCase
  fixtures :companies, :topics, :entrants, :developers, :developers_projects, :posts, :comments, :accounts, :authors, :customers, :categories, :categorizations

  def test_find_by_id_with_hash
    assert_raises(ActiveRecord::StatementInvalid) do
      Post.find_by_id(:limit => 1)
    end
  end

  def test_find_by_title_and_id_with_hash
    assert_raises(ActiveRecord::StatementInvalid) do
      Post.find_by_title_and_id('foo', :limit => 1)
    end
  end

  def test_find
    assert_equal(topics(:first).title, Topic.find(1).title)
  end
