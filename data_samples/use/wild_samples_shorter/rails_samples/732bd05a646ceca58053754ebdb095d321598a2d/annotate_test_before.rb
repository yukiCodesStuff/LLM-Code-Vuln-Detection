  def test_annotate_is_sanitized
    quoted_posts_id, quoted_posts = regexp_escape_table_name("posts.id"), regexp_escape_table_name("posts")

    assert_sql(%r{SELECT #{quoted_posts_id} FROM #{quoted_posts} /\* foo \*/}i) do
      posts = Post.select(:id).annotate("*/foo/*")
      assert posts.first
    end

    assert_sql(%r{SELECT #{quoted_posts_id} FROM #{quoted_posts} /\* foo \*/}i) do
      posts = Post.select(:id).annotate("**//foo//**")
      assert posts.first
    end

    assert_sql(%r{SELECT #{quoted_posts_id} FROM #{quoted_posts} /\* foo \*/ /\* bar \*/}i) do
      posts = Post.select(:id).annotate("*/foo/*").annotate("*/bar")
      assert posts.first
    end
