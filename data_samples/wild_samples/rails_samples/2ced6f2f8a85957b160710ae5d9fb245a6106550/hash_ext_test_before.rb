    expected_topic_hash = {
      :title => "The First Topic",
      :author_name => "David",
      :id => 1,
      :approved => false,
      :replies_count => 0,
      :replies_close_in => 2592000000,
      :written_on => Date.new(2003, 7, 16),
      :viewed_at => Time.utc(2003, 7, 16, 9, 28),
      :content => "Have a nice day",
      :author_email_address => "david@loudthinking.com",
      :parent_id => nil
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topics_xml)["topics"].first
  end

  def test_single_record_from_xml
    topic_xml = <<-EOT
      <topic>
        <title>The First Topic</title>
        <author-name>David</author-name>
        <id type="integer">1</id>
        <approved type="boolean"> true </approved>
        <replies-count type="integer">0</replies-count>
        <replies-close-in type="integer">2592000000</replies-close-in>
        <written-on type="date">2003-07-16</written-on>
        <viewed-at type="datetime">2003-07-16T09:28:00+0000</viewed-at>
        <content type="yaml">--- \n1: should be an integer\n:message: Have a nice day\narray: \n- should-have-dashes: true\n  should_have_underscores: true\n</content>
        <author-email-address>david@loudthinking.com</author-email-address>
        <parent-id></parent-id>
        <ad-revenue type="decimal">1.5</ad-revenue>
        <optimum-viewing-angle type="float">135</optimum-viewing-angle>
        <resident type="symbol">yes</resident>
      </topic>
    EOT

    expected_topic_hash = {
      :title => "The First Topic",
      :author_name => "David",
      :id => 1,
      :approved => true,
      :replies_count => 0,
      :replies_close_in => 2592000000,
      :written_on => Date.new(2003, 7, 16),
      :viewed_at => Time.utc(2003, 7, 16, 9, 28),
      :content => { :message => "Have a nice day", 1 => "should be an integer", "array" => [{ "should-have-dashes" => true, "should_have_underscores" => true }] },
      :author_email_address => "david@loudthinking.com",
      :parent_id => nil,
      :ad_revenue => BigDecimal("1.50"),
      :optimum_viewing_angle => 135.0,
      :resident => :yes
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topic_xml)["topic"]
  end

  def test_single_record_from_xml_with_nil_values
    topic_xml = <<-EOT
      <topic>
        <title></title>
        <id type="integer"></id>
        <approved type="boolean"></approved>
        <written-on type="date"></written-on>
        <viewed-at type="datetime"></viewed-at>
        <content type="yaml"></content>
        <parent-id></parent-id>
      </topic>
    EOT

    expected_topic_hash = {
      :title      => nil,
      :id         => nil,
      :approved   => nil,
      :written_on => nil,
      :viewed_at  => nil,
      :content    => nil,
      :parent_id  => nil
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topic_xml)["topic"]
  end

  def test_multiple_records_from_xml
    topics_xml = <<-EOT
      <topics type="array">
        <topic>
          <title>The First Topic</title>
          <author-name>David</author-name>
          <id type="integer">1</id>
          <approved type="boolean">false</approved>
          <replies-count type="integer">0</replies-count>
          <replies-close-in type="integer">2592000000</replies-close-in>
          <written-on type="date">2003-07-16</written-on>
          <viewed-at type="datetime">2003-07-16T09:28:00+0000</viewed-at>
          <content>Have a nice day</content>
          <author-email-address>david@loudthinking.com</author-email-address>
          <parent-id nil="true"></parent-id>
        </topic>
        <topic>
          <title>The Second Topic</title>
          <author-name>Jason</author-name>
          <id type="integer">1</id>
          <approved type="boolean">false</approved>
          <replies-count type="integer">0</replies-count>
          <replies-close-in type="integer">2592000000</replies-close-in>
          <written-on type="date">2003-07-16</written-on>
          <viewed-at type="datetime">2003-07-16T09:28:00+0000</viewed-at>
          <content>Have a nice day</content>
          <author-email-address>david@loudthinking.com</author-email-address>
          <parent-id></parent-id>
        </topic>
      </topics>
    EOT

    expected_topic_hash = {
      :title => "The First Topic",
      :author_name => "David",
      :id => 1,
      :approved => false,
      :replies_count => 0,
      :replies_close_in => 2592000000,
      :written_on => Date.new(2003, 7, 16),
      :viewed_at => Time.utc(2003, 7, 16, 9, 28),
      :content => "Have a nice day",
      :author_email_address => "david@loudthinking.com",
      :parent_id => nil
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topics_xml)["topics"].first
  end

  def test_single_record_from_xml_with_attributes_other_than_type
    topic_xml = <<-EOT
    <rsp stat="ok">
      <photos page="1" pages="1" perpage="100" total="16">
        <photo id="175756086" owner="55569174@N00" secret="0279bf37a1" server="76" title="Colored Pencil PhotoBooth Fun" ispublic="1" isfriend="0" isfamily="0"/>
      </photos>
    </rsp>
    EOT

    expected_topic_hash = {
      :id => "175756086",
      :owner => "55569174@N00",
      :secret => "0279bf37a1",
      :server => "76",
      :title => "Colored Pencil PhotoBooth Fun",
      :ispublic => "1",
      :isfriend => "0",
      :isfamily => "0",
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topic_xml)["rsp"]["photos"]["photo"]
  end

  def test_all_caps_key_from_xml
    test_xml = <<-EOT
      <ABC3XYZ>
        <TEST>Lorem Ipsum</TEST>
      </ABC3XYZ>
    EOT

    expected_hash = {
      "ABC3XYZ" => {
        "TEST" => "Lorem Ipsum"
      }
    }

    assert_equal expected_hash, Hash.from_xml(test_xml)
  end

  def test_empty_array_from_xml
    blog_xml = <<-XML
      <blog>
        <posts type="array"></posts>
      </blog>
    XML
    expected_blog_hash = {"blog" => {"posts" => []}}
    assert_equal expected_blog_hash, Hash.from_xml(blog_xml)
  end

  def test_empty_array_with_whitespace_from_xml
    blog_xml = <<-XML
      <blog>
        <posts type="array">
        </posts>
      </blog>
    XML
    expected_blog_hash = {"blog" => {"posts" => []}}
    assert_equal expected_blog_hash, Hash.from_xml(blog_xml)
  end

  def test_array_with_one_entry_from_xml
    blog_xml = <<-XML
      <blog>
        <posts type="array">
          <post>a post</post>
        </posts>
      </blog>
    XML
    expected_blog_hash = {"blog" => {"posts" => ["a post"]}}
    assert_equal expected_blog_hash, Hash.from_xml(blog_xml)
  end

  def test_array_with_multiple_entries_from_xml
    blog_xml = <<-XML
      <blog>
        <posts type="array">
          <post>a post</post>
          <post>another post</post>
        </posts>
      </blog>
    XML
    expected_blog_hash = {"blog" => {"posts" => ["a post", "another post"]}}
    expected_topic_hash = {
      :title => "The First Topic",
      :author_name => "David",
      :id => 1,
      :approved => true,
      :replies_count => 0,
      :replies_close_in => 2592000000,
      :written_on => Date.new(2003, 7, 16),
      :viewed_at => Time.utc(2003, 7, 16, 9, 28),
      :content => { :message => "Have a nice day", 1 => "should be an integer", "array" => [{ "should-have-dashes" => true, "should_have_underscores" => true }] },
      :author_email_address => "david@loudthinking.com",
      :parent_id => nil,
      :ad_revenue => BigDecimal("1.50"),
      :optimum_viewing_angle => 135.0,
      :resident => :yes
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topic_xml)["topic"]
  end

  def test_single_record_from_xml_with_nil_values
    topic_xml = <<-EOT
      <topic>
        <title></title>
        <id type="integer"></id>
        <approved type="boolean"></approved>
        <written-on type="date"></written-on>
        <viewed-at type="datetime"></viewed-at>
        <content type="yaml"></content>
        <parent-id></parent-id>
      </topic>
    EOT

    expected_topic_hash = {
      :title      => nil,
      :id         => nil,
      :approved   => nil,
      :written_on => nil,
      :viewed_at  => nil,
      :content    => nil,
      :parent_id  => nil
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topic_xml)["topic"]
  end

  def test_multiple_records_from_xml
    topics_xml = <<-EOT
      <topics type="array">
        <topic>
          <title>The First Topic</title>
          <author-name>David</author-name>
          <id type="integer">1</id>
          <approved type="boolean">false</approved>
          <replies-count type="integer">0</replies-count>
          <replies-close-in type="integer">2592000000</replies-close-in>
          <written-on type="date">2003-07-16</written-on>
          <viewed-at type="datetime">2003-07-16T09:28:00+0000</viewed-at>
          <content>Have a nice day</content>
          <author-email-address>david@loudthinking.com</author-email-address>
          <parent-id nil="true"></parent-id>
        </topic>
        <topic>
          <title>The Second Topic</title>
          <author-name>Jason</author-name>
          <id type="integer">1</id>
          <approved type="boolean">false</approved>
          <replies-count type="integer">0</replies-count>
          <replies-close-in type="integer">2592000000</replies-close-in>
          <written-on type="date">2003-07-16</written-on>
          <viewed-at type="datetime">2003-07-16T09:28:00+0000</viewed-at>
          <content>Have a nice day</content>
          <author-email-address>david@loudthinking.com</author-email-address>
          <parent-id></parent-id>
        </topic>
      </topics>
    EOT

    expected_topic_hash = {
      :title => "The First Topic",
      :author_name => "David",
      :id => 1,
      :approved => false,
      :replies_count => 0,
      :replies_close_in => 2592000000,
      :written_on => Date.new(2003, 7, 16),
      :viewed_at => Time.utc(2003, 7, 16, 9, 28),
      :content => "Have a nice day",
      :author_email_address => "david@loudthinking.com",
      :parent_id => nil
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topics_xml)["topics"].first
  end

  def test_single_record_from_xml_with_attributes_other_than_type
    topic_xml = <<-EOT
    <rsp stat="ok">
      <photos page="1" pages="1" perpage="100" total="16">
        <photo id="175756086" owner="55569174@N00" secret="0279bf37a1" server="76" title="Colored Pencil PhotoBooth Fun" ispublic="1" isfriend="0" isfamily="0"/>
      </photos>
    </rsp>
    EOT

    expected_topic_hash = {
      :id => "175756086",
      :owner => "55569174@N00",
      :secret => "0279bf37a1",
      :server => "76",
      :title => "Colored Pencil PhotoBooth Fun",
      :ispublic => "1",
      :isfriend => "0",
      :isfamily => "0",
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topic_xml)["rsp"]["photos"]["photo"]
  end

  def test_all_caps_key_from_xml
    test_xml = <<-EOT
      <ABC3XYZ>
        <TEST>Lorem Ipsum</TEST>
      </ABC3XYZ>
    EOT

    expected_hash = {
      "ABC3XYZ" => {
        "TEST" => "Lorem Ipsum"
      }
    }

    assert_equal expected_hash, Hash.from_xml(test_xml)
  end

  def test_empty_array_from_xml
    blog_xml = <<-XML
      <blog>
        <posts type="array"></posts>
      </blog>
    XML
    expected_blog_hash = {"blog" => {"posts" => []}}
    assert_equal expected_blog_hash, Hash.from_xml(blog_xml)
  end

  def test_empty_array_with_whitespace_from_xml
    blog_xml = <<-XML
      <blog>
        <posts type="array">
        </posts>
      </blog>
    XML
    expected_blog_hash = {"blog" => {"posts" => []}}
    assert_equal expected_blog_hash, Hash.from_xml(blog_xml)
  end

  def test_array_with_one_entry_from_xml
    blog_xml = <<-XML
      <blog>
        <posts type="array">
          <post>a post</post>
        </posts>
      </blog>
    XML
    expected_blog_hash = {"blog" => {"posts" => ["a post"]}}
    expected_topic_hash = {
      :title => "The First Topic",
      :author_name => "David",
      :id => 1,
      :approved => true,
      :replies_count => 0,
      :replies_close_in => 2592000000,
      :written_on => Date.new(2003, 7, 16),
      :viewed_at => Time.utc(2003, 7, 16, 9, 28),
      :content => { :message => "Have a nice day", 1 => "should be an integer", "array" => [{ "should-have-dashes" => true, "should_have_underscores" => true }] },
      :author_email_address => "david@loudthinking.com",
      :parent_id => nil,
      :ad_revenue => BigDecimal("1.50"),
      :optimum_viewing_angle => 135.0,
      :resident => :yes
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topic_xml)["topic"]
  end

  def test_single_record_from_xml_with_nil_values
    topic_xml = <<-EOT
      <topic>
        <title></title>
        <id type="integer"></id>
        <approved type="boolean"></approved>
        <written-on type="date"></written-on>
        <viewed-at type="datetime"></viewed-at>
        <content type="yaml"></content>
        <parent-id></parent-id>
      </topic>
    EOT

    expected_topic_hash = {
      :title      => nil,
      :id         => nil,
      :approved   => nil,
      :written_on => nil,
      :viewed_at  => nil,
      :content    => nil,
      :parent_id  => nil
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topic_xml)["topic"]
  end

  def test_multiple_records_from_xml
    topics_xml = <<-EOT
      <topics type="array">
        <topic>
          <title>The First Topic</title>
          <author-name>David</author-name>
          <id type="integer">1</id>
          <approved type="boolean">false</approved>
          <replies-count type="integer">0</replies-count>
          <replies-close-in type="integer">2592000000</replies-close-in>
          <written-on type="date">2003-07-16</written-on>
          <viewed-at type="datetime">2003-07-16T09:28:00+0000</viewed-at>
          <content>Have a nice day</content>
          <author-email-address>david@loudthinking.com</author-email-address>
          <parent-id nil="true"></parent-id>
        </topic>
        <topic>
          <title>The Second Topic</title>
          <author-name>Jason</author-name>
          <id type="integer">1</id>
          <approved type="boolean">false</approved>
          <replies-count type="integer">0</replies-count>
          <replies-close-in type="integer">2592000000</replies-close-in>
          <written-on type="date">2003-07-16</written-on>
          <viewed-at type="datetime">2003-07-16T09:28:00+0000</viewed-at>
          <content>Have a nice day</content>
          <author-email-address>david@loudthinking.com</author-email-address>
          <parent-id></parent-id>
        </topic>
      </topics>
    EOT

    expected_topic_hash = {
      :title => "The First Topic",
      :author_name => "David",
      :id => 1,
      :approved => false,
      :replies_count => 0,
      :replies_close_in => 2592000000,
      :written_on => Date.new(2003, 7, 16),
      :viewed_at => Time.utc(2003, 7, 16, 9, 28),
      :content => "Have a nice day",
      :author_email_address => "david@loudthinking.com",
      :parent_id => nil
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topics_xml)["topics"].first
  end

  def test_single_record_from_xml_with_attributes_other_than_type
    topic_xml = <<-EOT
    <rsp stat="ok">
      <photos page="1" pages="1" perpage="100" total="16">
        <photo id="175756086" owner="55569174@N00" secret="0279bf37a1" server="76" title="Colored Pencil PhotoBooth Fun" ispublic="1" isfriend="0" isfamily="0"/>
      </photos>
    </rsp>
    EOT

    expected_topic_hash = {
      :id => "175756086",
      :owner => "55569174@N00",
      :secret => "0279bf37a1",
      :server => "76",
      :title => "Colored Pencil PhotoBooth Fun",
      :ispublic => "1",
      :isfriend => "0",
      :isfamily => "0",
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topic_xml)["rsp"]["photos"]["photo"]
  end

  def test_all_caps_key_from_xml
    test_xml = <<-EOT
      <ABC3XYZ>
        <TEST>Lorem Ipsum</TEST>
      </ABC3XYZ>
    EOT

    expected_hash = {
      "ABC3XYZ" => {
        "TEST" => "Lorem Ipsum"
      }
    }

    assert_equal expected_hash, Hash.from_xml(test_xml)
  end

  def test_empty_array_from_xml
    blog_xml = <<-XML
      <blog>
        <posts type="array"></posts>
      </blog>
    XML
    expected_blog_hash = {"blog" => {"posts" => []}}
    assert_equal expected_blog_hash, Hash.from_xml(blog_xml)
  end

  def test_empty_array_with_whitespace_from_xml
    blog_xml = <<-XML
      <blog>
        <posts type="array">
        </posts>
      </blog>
    XML
    expected_blog_hash = {"blog" => {"posts" => []}}
    assert_equal expected_blog_hash, Hash.from_xml(blog_xml)
  end

  def test_array_with_one_entry_from_xml
    blog_xml = <<-XML
      <blog>
        <posts type="array">
          <post>a post</post>
        </posts>
      </blog>
    XML
    expected_blog_hash = {"blog" => {"posts" => ["a post"]}}
    expected_topic_hash = {
      :title      => nil,
      :id         => nil,
      :approved   => nil,
      :written_on => nil,
      :viewed_at  => nil,
      :content    => nil,
      :parent_id  => nil
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topic_xml)["topic"]
  end

  def test_multiple_records_from_xml
    topics_xml = <<-EOT
      <topics type="array">
        <topic>
          <title>The First Topic</title>
          <author-name>David</author-name>
          <id type="integer">1</id>
          <approved type="boolean">false</approved>
          <replies-count type="integer">0</replies-count>
          <replies-close-in type="integer">2592000000</replies-close-in>
          <written-on type="date">2003-07-16</written-on>
          <viewed-at type="datetime">2003-07-16T09:28:00+0000</viewed-at>
          <content>Have a nice day</content>
          <author-email-address>david@loudthinking.com</author-email-address>
          <parent-id nil="true"></parent-id>
        </topic>
        <topic>
          <title>The Second Topic</title>
          <author-name>Jason</author-name>
          <id type="integer">1</id>
          <approved type="boolean">false</approved>
          <replies-count type="integer">0</replies-count>
          <replies-close-in type="integer">2592000000</replies-close-in>
          <written-on type="date">2003-07-16</written-on>
          <viewed-at type="datetime">2003-07-16T09:28:00+0000</viewed-at>
          <content>Have a nice day</content>
          <author-email-address>david@loudthinking.com</author-email-address>
          <parent-id></parent-id>
        </topic>
      </topics>
    EOT

    expected_topic_hash = {
      :title => "The First Topic",
      :author_name => "David",
      :id => 1,
      :approved => false,
      :replies_count => 0,
      :replies_close_in => 2592000000,
      :written_on => Date.new(2003, 7, 16),
      :viewed_at => Time.utc(2003, 7, 16, 9, 28),
      :content => "Have a nice day",
      :author_email_address => "david@loudthinking.com",
      :parent_id => nil
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topics_xml)["topics"].first
  end

  def test_single_record_from_xml_with_attributes_other_than_type
    topic_xml = <<-EOT
    <rsp stat="ok">
      <photos page="1" pages="1" perpage="100" total="16">
        <photo id="175756086" owner="55569174@N00" secret="0279bf37a1" server="76" title="Colored Pencil PhotoBooth Fun" ispublic="1" isfriend="0" isfamily="0"/>
      </photos>
    </rsp>
    EOT

    expected_topic_hash = {
      :id => "175756086",
      :owner => "55569174@N00",
      :secret => "0279bf37a1",
      :server => "76",
      :title => "Colored Pencil PhotoBooth Fun",
      :ispublic => "1",
      :isfriend => "0",
      :isfamily => "0",
    }.stringify_keys

    assert_equal expected_topic_hash, Hash.from_xml(topic_xml)["rsp"]["photos"]["photo"]
  end

  def test_all_caps_key_from_xml
    test_xml = <<-EOT
      <ABC3XYZ>
        <TEST>Lorem Ipsum</TEST>
      </ABC3XYZ>
    EOT

    expected_hash = {
      "ABC3XYZ" => {
        "TEST" => "Lorem Ipsum"
      }
    }

    assert_equal expected_hash, Hash.from_xml(test_xml)
  end

  def test_empty_array_from_xml
    blog_xml = <<-XML
      <blog>
        <posts type="array"></posts>
      </blog>
    XML
    expected_blog_hash = {"blog" => {"posts" => []}}
    assert_equal expected_blog_hash, Hash.from_xml(blog_xml)
  end

  def test_empty_array_with_whitespace_from_xml
    blog_xml = <<-XML
      <blog>
        <posts type="array">
        </posts>
      </blog>
    XML
    expected_blog_hash = {"blog" => {"posts" => []}}
    expected_product_hash = {
      :weight => 0.5,
      :image => {'type' => 'ProductImage', 'filename' => 'image.gif' },
    }.stringify_keys

    assert_equal expected_product_hash, Hash.from_xml(product_xml)["product"]
  end

  def test_should_use_default_value_for_unknown_key
    hash_wia = HashWithIndifferentAccess.new(3)
    assert_equal 3, hash_wia[:new_key]
  end

  def test_should_use_default_value_if_no_key_is_supplied
    hash_wia = HashWithIndifferentAccess.new(3)
    assert_equal 3, hash_wia.default
  end

  def test_should_nil_if_no_default_value_is_supplied
    hash_wia = HashWithIndifferentAccess.new
    assert_nil hash_wia.default
  end

  def test_should_return_dup_for_with_indifferent_access
    hash_wia = HashWithIndifferentAccess.new
    assert_equal hash_wia, hash_wia.with_indifferent_access
    assert_not_same hash_wia, hash_wia.with_indifferent_access
  end

  def test_should_copy_the_default_value_when_converting_to_hash_with_indifferent_access
    hash = Hash.new(3)
    hash_wia = hash.with_indifferent_access
    assert_equal 3, hash_wia.default
  end

  # The XML builder seems to fail miserably when trying to tag something
  # with the same name as a Kernel method (throw, test, loop, select ...)
  def test_kernel_method_names_to_xml
    hash     = { :throw => { :ball => 'red' } }
    expected = '<person><throw><ball>red</ball></throw></person>'

    assert_nothing_raised do
      assert_equal expected, hash.to_xml(@xml_options)
    end
  end

  def test_empty_string_works_for_typecast_xml_value
    assert_nothing_raised do
      ActiveSupport::XMLConverter.new("").to_h
    end
  end

  def test_escaping_to_xml
    hash = {
      :bare_string        => 'First & Last Name',
      :pre_escaped_string => 'First &amp; Last Name'
    }.stringify_keys

    expected_xml = '<person><bare-string>First &amp; Last Name</bare-string><pre-escaped-string>First &amp;amp; Last Name</pre-escaped-string></person>'
    assert_equal expected_xml, hash.to_xml(@xml_options)
  end

  def test_unescaping_from_xml
    xml_string = '<person><bare-string>First &amp; Last Name</bare-string><pre-escaped-string>First &amp;amp; Last Name</pre-escaped-string></person>'
    expected_hash = {
      :bare_string        => 'First & Last Name',
      :pre_escaped_string => 'First &amp; Last Name'
    }.stringify_keys
    assert_equal expected_hash, Hash.from_xml(xml_string)['person']
  end

  def test_roundtrip_to_xml_from_xml
    hash = {
      :bare_string        => 'First & Last Name',
      :pre_escaped_string => 'First &amp; Last Name'
    }.stringify_keys

    assert_equal hash, Hash.from_xml(hash.to_xml(@xml_options))['person']
  end

  def test_datetime_xml_type_with_utc_time
    alert_xml = <<-XML
      <alert>
        <alert_at type="datetime">2008-02-10T15:30:45Z</alert_at>
      </alert>
    XML
    alert_at = Hash.from_xml(alert_xml)['alert']['alert_at']
    assert alert_at.utc?
    assert_equal Time.utc(2008, 2, 10, 15, 30, 45), alert_at
  end

  def test_datetime_xml_type_with_non_utc_time
    alert_xml = <<-XML
      <alert>
        <alert_at type="datetime">2008-02-10T10:30:45-05:00</alert_at>
      </alert>
    XML
    alert_at = Hash.from_xml(alert_xml)['alert']['alert_at']
    assert alert_at.utc?
    assert_equal Time.utc(2008, 2, 10, 15, 30, 45), alert_at
  end

  def test_datetime_xml_type_with_far_future_date
    alert_xml = <<-XML
      <alert>
        <alert_at type="datetime">2050-02-10T15:30:45Z</alert_at>
      </alert>
    XML
    alert_at = Hash.from_xml(alert_xml)['alert']['alert_at']
    assert alert_at.utc?
    assert_equal 2050,  alert_at.year
    assert_equal 2,     alert_at.month
    assert_equal 10,    alert_at.day
    assert_equal 15,    alert_at.hour
    assert_equal 30,    alert_at.min
    assert_equal 45,    alert_at.sec
  end

  def test_to_xml_dups_options
    options = {:skip_instruct => true}
    {}.to_xml(options)
    # :builder, etc, shouldn't be added to options
    assert_equal({:skip_instruct => true}, options)
  end

  def test_expansion_count_is_limited
    expected =
      case ActiveSupport::XmlMini.backend.name
      when 'ActiveSupport::XmlMini_REXML';        RuntimeError
      when 'ActiveSupport::XmlMini_Nokogiri';     Nokogiri::XML::SyntaxError
      when 'ActiveSupport::XmlMini_NokogiriSAX';  RuntimeError
      when 'ActiveSupport::XmlMini_LibXML';       LibXML::XML::Error
      when 'ActiveSupport::XmlMini_LibXMLSAX';    LibXML::XML::Error
      end

    assert_raise expected do
      attack_xml = <<-EOT
      <?xml version="1.0" encoding="UTF-8"?>
      <!DOCTYPE member [
        <!ENTITY a "&b;&b;&b;&b;&b;&b;&b;&b;&b;&b;">
        <!ENTITY b "&c;&c;&c;&c;&c;&c;&c;&c;&c;&c;">
        <!ENTITY c "&d;&d;&d;&d;&d;&d;&d;&d;&d;&d;">
        <!ENTITY d "&e;&e;&e;&e;&e;&e;&e;&e;&e;&e;">
        <!ENTITY e "&f;&f;&f;&f;&f;&f;&f;&f;&f;&f;">
        <!ENTITY f "&g;&g;&g;&g;&g;&g;&g;&g;&g;&g;">
        <!ENTITY g "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx">
      ]>
      <member>
      &a;
      </member>
      EOT
      Hash.from_xml(attack_xml)
    end
  end
end