
class FallbackFileSystemResolverTest < ActiveSupport::TestCase
  def setup
    @root_resolver = ActionView::FallbackFileSystemResolver.new("/")
  end

  def test_should_have_no_virtual_path
    templates = @root_resolver.find_all("hello_world.erb", "#{FIXTURE_LOAD_PATH}/test", false, locale: [], formats: [:html], variants: [], handlers: [:erb])