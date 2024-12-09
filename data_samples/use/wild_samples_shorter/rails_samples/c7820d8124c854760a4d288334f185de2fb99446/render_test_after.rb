    assert_match(/You invoked render but did not give any of (.+) option\./, e.message)
  end

  def test_render_template
    assert_equal "Hello world!", @view.render(template: "test/hello_world")
  end


  def test_render_file
    assert_equal "Hello world!", assert_deprecated { @view.render(file: "test/hello_world") }
  end

  # Test if :formats, :locale etc. options are passed correctly to the resolvers.
  def test_render_file_with_format
    assert_match "<h1>No Comment</h1>", assert_deprecated { @view.render(file: "comments/empty", formats: [:html]) }
    assert_match "<error>No Comment</error>", assert_deprecated { @view.render(file: "comments/empty", formats: [:xml]) }
    assert_match "<error>No Comment</error>", assert_deprecated { @view.render(file: "comments/empty", formats: :xml) }
  end

  def test_render_template_with_format
    assert_match "<h1>No Comment</h1>", @view.render(template: "comments/empty", formats: [:html])
  end

  def test_render_file_with_locale
    assert_equal "<h1>Kein Kommentar</h1>", assert_deprecated { @view.render(file: "comments/empty", locale: [:de]) }
    assert_equal "<h1>Kein Kommentar</h1>", assert_deprecated { @view.render(file: "comments/empty", locale: :de) }
  end

  def test_render_template_with_locale
    assert_equal "<h1>Kein Kommentar</h1>", @view.render(template: "comments/empty", locale: [:de])
  end

  def test_render_file_with_handlers
    assert_equal "<h1>No Comment</h1>\n", assert_deprecated { @view.render(file: "comments/empty", handlers: [:builder]) }
    assert_equal "<h1>No Comment</h1>\n", assert_deprecated { @view.render(file: "comments/empty", handlers: :builder) }
  end

  def test_render_template_with_handlers
    assert_equal "<h1>No Comment</h1>\n", @view.render(template: "comments/empty", handlers: [:builder])
    assert_equal "Elastica", @view.render(template: "/shared")
  end

  def test_render_file_with_full_path_no_extension
    template_path = File.expand_path("../fixtures/test/hello_world", __dir__)
    assert_equal "Hello world!", assert_deprecated { @view.render(file: template_path) }
  end

  def test_render_file_with_full_path
    template_path = File.expand_path("../fixtures/test/hello_world.erb", __dir__)
    assert_equal "Hello world!", @view.render(file: template_path)
  end

  def test_render_file_with_instance_variables
    assert_equal "The secret is in the sauce\n", assert_deprecated { @view.render(file: "test/render_file_with_ivar") }
  end

  def test_render_file_with_locals
    locals = { secret: "in the sauce" }
    assert_equal "The secret is in the sauce\n", assert_deprecated { @view.render(file: "test/render_file_with_locals", locals: locals) }
  end

  def test_render_file_not_using_full_path_with_dot_in_path
    assert_equal "The secret is in the sauce\n", assert_deprecated { @view.render(file: "test/dot.directory/render_file_with_ivar") }
  end

  def test_render_partial_from_default
    assert_equal "only partial", @view.render("test/partial_only")
  end

  def test_render_file_with_errors
    e = assert_raises(ActionView::Template::Error) { assert_deprecated { @view.render(file: File.expand_path("test/_raise", FIXTURE_LOAD_PATH)) } }
    assert_match %r!method.*doesnt_exist!, e.message
    assert_equal "", e.sub_template_message
    assert_equal "1", e.line_number
    assert_equal "1: <%= doesnt_exist %>", e.annoted_source_code[0].strip