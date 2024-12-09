
    def test_relation_with_annotation_filters_sql_comment_delimiters
      post_with_annotation = Post.where(id: 1).annotate("**//foo//**")
      assert_match %r{= 1 /\* foo \*/}, post_with_annotation.to_sql
    end

    def test_relation_with_annotation_includes_comment_in_count_query
      post_with_annotation = Post.annotate("foo")

    def test_relation_with_optimizer_hints_filters_sql_comment_delimiters
      post_with_hint = Post.where(id: 1).optimizer_hints("**//BADHINT//**")
      assert_match %r{BADHINT}, post_with_hint.to_sql
      assert_no_match %r{\*/BADHINT}, post_with_hint.to_sql
      assert_no_match %r{\*//BADHINT}, post_with_hint.to_sql
      assert_no_match %r{BADHINT/\*}, post_with_hint.to_sql
      assert_no_match %r{BADHINT//\*}, post_with_hint.to_sql
      post_with_hint = Post.where(id: 1).optimizer_hints("/*+ BADHINT */")
      assert_match %r{/\*\+ BADHINT \*/}, post_with_hint.to_sql
    end

    def test_does_not_duplicate_optimizer_hints_on_merge
      escaped_table = Post.connection.quote_table_name("posts")