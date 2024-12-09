config.action_mailer.default_options = {
  from: "noreply@example.com"
}
```

#### `config.action_mailer.observers`

Registers observers which will be notified when mail is delivered.

```ruby
config.action_mailer.observers = ["MailObserver"]
```

#### `config.action_mailer.interceptors`

Registers interceptors which will be called before mail is sent.

```ruby
config.action_mailer.interceptors = ["MailInterceptor"]
```

#### `config.action_mailer.preview_interceptors`

Registers interceptors which will be called before mail is previewed.

```ruby
config.action_mailer.preview_interceptors = ["MyPreviewMailInterceptor"]
```

#### `config.action_mailer.preview_path`

Specifies the location of mailer previews.

```ruby
config.action_mailer.preview_path = "#{Rails.root}/lib/mailer_previews"
```

#### `config.action_mailer.show_previews`

Enable or disable mailer previews. By default this is `true` in development.

```ruby
config.action_mailer.show_previews = false
```

#### `config.action_mailer.deliver_later_queue_name`

Specifies the Active Job queue to use for delivery jobs. When this option is set to `nil`, delivery jobs are sent to the default Active Job queue (see `config.active_job.default_queue_name`). Make sure that your Active Job adapter is also configured to process the specified queue, otherwise delivery jobs may be silently ignored.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `:mailers`           |
| 6.1                   | `nil`                |

#### `config.action_mailer.perform_caching`

Specifies whether the mailer templates should perform fragment caching or not. If it's not specified, the default will be `true`.

#### `config.action_mailer.delivery_job`

Specifies delivery job for mail.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `ActionMailer::MailDeliveryJob` |
| 6.0                   | `"ActionMailer::MailDeliveryJob"` |

### Configuring Active Support

There are a few configuration options available in Active Support:

#### `config.active_support.bare`

Enables or disables the loading of `active_support/all` when booting Rails. Defaults to `nil`, which means `active_support/all` is loaded.

#### `config.active_support.test_order`

Sets the order in which the test cases are executed. Possible values are `:random` and `:sorted`. Defaults to `:random`.

#### `config.active_support.escape_html_entities_in_json`

Enables or disables the escaping of HTML entities in JSON serialization. Defaults to `true`.

#### `config.active_support.use_standard_json_time_format`

Enables or disables serializing dates to ISO 8601 format. Defaults to `true`.

#### `config.active_support.time_precision`

Sets the precision of JSON encoded time values. Defaults to `3`.

#### `config.active_support.hash_digest_class`

Allows configuring the digest class to use to generate non-sensitive digests, such as the ETag header.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `OpenSSL::Digest::MD5` |
| 5.2                   | `OpenSSL::Digest::SHA1` |
| 7.0                   | `OpenSSL::Digest::SHA256` |

#### `config.active_support.key_generator_hash_digest_class`

Allows configuring the digest class to use to derive secrets from the configured secret base, such as for encrypted cookies.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `OpenSSL::Digest::SHA1` |
| 7.0                   | `OpenSSL::Digest::SHA256` |

#### `config.active_support.use_authenticated_message_encryption`

Specifies whether to use AES-256-GCM authenticated encryption as the default cipher for encrypting messages instead of AES-256-CBC.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 5.2                   | `true`               |

#### `config.active_support.cache_format_version`

Specifies which version of the cache serializer to use. Possible values are `6.1` and `7.0`.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `6.1`                |
| 7.0                   | `7.0`                |

#### `config.active_support.deprecation`

Configures the behavior of deprecation warnings. The options are `:raise`, `:stderr`, `:log`, `:notify`, or `:silence`. The default is `:stderr`. Alternatively, you can set `ActiveSupport::Deprecation.behavior`.

#### `config.active_support.disallowed_deprecation`

Configures the behavior of disallowed deprecation warnings. The options are `:raise`, `:stderr`, `:log`, `:notify`, or `:silence`. The default is `:raise`. Alternatively, you can set `ActiveSupport::Deprecation.disallowed_behavior`.

#### `config.active_support.disallowed_deprecation_warnings`

Configures deprecation warnings that the Application considers disallowed. This allows, for example, specific deprecations to be treated as hard failures. Alternatively, you can set `ActiveSupport::Deprecation.disallowed_warnings`.

#### `config.active_support.report_deprecations`

Allows you to disable all deprecation warnings (including disallowed deprecations); it makes `ActiveSupport::Deprecation.warn` a no-op. This is enabled by default in production.

#### `config.active_support.remove_deprecated_time_with_zone_name`

Specifies whether to remove the deprecated override of the [`ActiveSupport::TimeWithZone.name`](https://api.rubyonrails.org/classes/ActiveSupport/TimeWithZone.html#method-c-name) method, to avoid triggering its deprecation warning.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `nil`                |
| 7.0                   | `true`               |

#### `config.active_support.isolation_level`

Configures the locality of most of Rails internal state. If you use a fiber based server or job processor (e.g. `falcon`), you should set it to `:fiber`. Otherwise it is best to use `:thread` locality. Defaults to `:thread`.

#### `config.active_support.use_rfc4122_namespaced_uuids`

Specifies whether generated namespaced UUIDs follow the RFC 4122 standard for namespace IDs provided as a `String` to `Digest::UUID.uuid_v3` or `Digest::UUID.uuid_v5` method calls.

If set to `true`:

* Only UUIDs are allowed as namespace IDs. If a namespace ID value provided is not allowed, an `ArgumentError` will be raised.
* No deprecation warning will be generated, no matter if the namespace ID used is one of the constants defined on `Digest::UUID` or a `String`.
* Namespace IDs are case-insensitive.
* All generated namespaced UUIDs should be compliant to the standard.

If set to `false`:

* Any `String` value can be used as namespace ID (although not recommended). No `ArgumentError` will be raised in this case in order to preserve backwards-compatibility.
* A deprecation warning will be generated if the namespace ID provided is not one of the constants defined on `Digest::UUID`.
* Namespace IDs are case-sensitive.
* Only namespaced UUIDs generated using one of the namespace ID constants defined on `Digest::UUID` are compliant to the standard.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 7.0                   | `true`               |

#### `config.active_support.executor_around_test_case`

Configure the test suite to call `Rails.application.executor.wrap` around test cases.
This makes test cases behave closer to an actual request or job.
Several features that are normally disabled in test, such as Active Record query cache
and asynchronous queries will then be enabled.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 7.0                   | `true`               |

#### `config.active_support.disable_to_s_conversion`

Disables the override of the `#to_s` methods in some Ruby core classes. This config is for applications that want to
take advantage early of a [Ruby 3.1 optimization](https://github.com/ruby/ruby/commit/b08dacfea39ad8da3f1fd7fdd0e4538cc892ec44).
This configuration needs to be set in `config/application.rb` inside the application class, otherwise it will not take
effect.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 7.0                   | `true`               |

#### `ActiveSupport::Logger.silencer`

Is set to `false` to disable the ability to silence logging in a block. The default is `true`.

#### `ActiveSupport::Cache::Store.logger`

Specifies the logger to use within cache store operations.

#### `ActiveSupport.to_time_preserves_timezone`

Specifies whether `to_time` methods preserve the UTC offset of their receivers. If `false`, `to_time` methods will convert to the local system UTC offset instead.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 5.0                   | `true`               |

#### `ActiveSupport.utc_to_local_returns_utc_offset_times`

Configures `ActiveSupport::TimeZone.utc_to_local` to return a time with a UTC
offset instead of a UTC time incorporating that offset.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 6.1                   | `true`               |

#### `config.active_support.default_message_encryptor_serializer`

Specifies what serializer the `MessageEncryptor` class will use by default.

Options are `:json`, `:hybrid`, and `:marshal`. `:hybrid` uses the `JsonWithMarshalFallback` class.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `:marshal`           |
| 7.1                   | `:json`              |

#### `config.active_support.fallback_to_marshal_deserialization`

Specifies if the `ActiveSupport::JsonWithMarshalFallback` class will fallback to `Marshal` when it encounters a `::JSON::ParserError`.

Defaults to `true`.

#### `config.active_support.use_marshal_serialization`

Specifies if the `ActiveSupport::JsonWithMarshalFallback` class will use `Marshal` to serialize payloads.

If this is set to `false`, it will use `JSON` to serialize payloads.

Used to help migrate apps from `Marshal` to `JSON` as the default serializer for the `MessageEncryptor` class.

Defaults to `true`.

#### `config.active_support.default_message_verifier_serializer`

Specifies what serializer the `MessageVerifier` class will use by default.

Options are `:json`, `:hybrid`, and `:marshal`. `:hybrid` uses the `JsonWithMarshalFallback` class.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `:marshal`           |
| 7.1                   | `:json`              |

### Configuring Active Job

`config.active_job` provides the following configuration options:

#### `config.active_job.queue_adapter`

Sets the adapter for the queuing backend. The default adapter is `:async`. For an up-to-date list of built-in adapters see the [ActiveJob::QueueAdapters API documentation](https://api.rubyonrails.org/classes/ActiveJob/QueueAdapters.html).

```ruby
# Be sure to have the adapter's gem in your Gemfile
# and follow the adapter's specific installation
# and deployment instructions.
config.active_job.queue_adapter = :sidekiq
```

#### `config.active_job.default_queue_name`

Can be used to change the default queue name. By default this is `"default"`.

```ruby
config.active_job.default_queue_name = :medium_priority
```

#### `config.active_job.queue_name_prefix`

Allows you to set an optional, non-blank, queue name prefix for all jobs. By default it is blank and not used.

The following configuration would queue the given job on the `production_high_priority` queue when run in production:

```ruby
config.active_job.queue_name_prefix = Rails.env
```

```ruby
class GuestsCleanupJob < ActiveJob::Base
  queue_as :high_priority
  #....
end
```

#### `config.active_job.queue_name_delimiter`

Has a default value of `'_'`. If `queue_name_prefix` is set, then `queue_name_delimiter` joins the prefix and the non-prefixed queue name.

The following configuration would queue the provided job on the `video_server.low_priority` queue:

```ruby
# prefix must be set for delimiter to be used
config.active_job.queue_name_prefix = 'video_server'
config.active_job.queue_name_delimiter = '.'
```

```ruby
class EncoderJob < ActiveJob::Base
  queue_as :low_priority
  #....
end
```

#### `config.active_job.logger`

Accepts a logger conforming to the interface of Log4r or the default Ruby Logger class, which is then used to log information from Active Job. You can retrieve this logger by calling `logger` on either an Active Job class or an Active Job instance. Set to `nil` to disable logging.

#### `config.active_job.custom_serializers`

Allows to set custom argument serializers. Defaults to `[]`.

#### `config.active_job.log_arguments`

Controls if the arguments of a job are logged. Defaults to `true`.

#### `config.active_job.retry_jitter`

Controls the amount of "jitter" (random variation) applied to the delay time calculated when retrying failed jobs.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `0.0`                |
| 6.1                   | `0.15`               |

#### `config.active_job.log_query_tags_around_perform`

Determines whether job context for query tags will be automatically updated via
an `around_perform`. The default value is `true`.

### Configuring Action Cable

#### `config.action_cable.url`

Accepts a string for the URL for where you are hosting your Action Cable
server. You would use this option if you are running Action Cable servers that
are separated from your main application.

#### `config.action_cable.mount_path`

Accepts a string for where to mount Action Cable, as part of the main server
process. Defaults to `/cable`. You can set this as nil to not mount Action
Cable as part of your normal Rails server.

You can find more detailed configuration options in the
[Action Cable Overview](action_cable_overview.html#configuration).

#### `config.action_cable.precompile_assets`

Determines whether the Action Cable assets should be added to the asset pipeline precompilation. It
has no effect if Sprockets is not used. The default value is `true`.

### Configuring Active Storage

`config.active_storage` provides the following configuration options:

#### `config.active_storage.variant_processor`

Accepts a symbol `:mini_magick` or `:vips`, specifying whether variant transformations and blob analysis will be performed with MiniMagick or ruby-vips.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `:mini_magick`       |
| 7.0                   | `:vips`              |

#### `config.active_storage.analyzers`

Accepts an array of classes indicating the analyzers available for Active Storage blobs.
By default, this is defined as:

```ruby
config.active_storage.analyzers = [ActiveStorage::Analyzer::ImageAnalyzer::Vips, ActiveStorage::Analyzer::ImageAnalyzer::ImageMagick, ActiveStorage::Analyzer::VideoAnalyzer, ActiveStorage::Analyzer::AudioAnalyzer]
```

The image analyzers can extract width and height of an image blob; the video analyzer can extract width, height, duration, angle, aspect ratio, and presence/absence of video/audio channels of a video blob; the audio analyzer can extract duration and bit rate of an audio blob.

#### `config.active_storage.previewers`

Accepts an array of classes indicating the image previewers available in Active Storage blobs.
By default, this is defined as:

```ruby
config.active_storage.previewers = [ActiveStorage::Previewer::PopplerPDFPreviewer, ActiveStorage::Previewer::MuPDFPreviewer, ActiveStorage::Previewer::VideoPreviewer]
```

`PopplerPDFPreviewer` and `MuPDFPreviewer` can generate a thumbnail from the first page of a PDF blob; `VideoPreviewer` from the relevant frame of a video blob.

#### `config.active_storage.paths`

Accepts a hash of options indicating the locations of previewer/analyzer commands. The default is `{}`, meaning the commands will be looked for in the default path. Can include any of these options:

* `:ffprobe` - The location of the ffprobe executable.
* `:mutool` - The location of the mutool executable.
* `:ffmpeg` - The location of the ffmpeg executable.

```ruby
config.active_storage.paths[:ffprobe] = '/usr/local/bin/ffprobe'
```

#### `config.active_storage.variable_content_types`

Accepts an array of strings indicating the content types that Active Storage
can transform through ImageMagick.
By default, this is defined as:

```ruby
config.active_storage.variable_content_types = %w(image/png image/gif image/jpeg image/tiff image/bmp image/vnd.adobe.photoshop image/vnd.microsoft.icon image/webp image/avif image/heic image/heif)
```

#### `config.active_storage.web_image_content_types`

Accepts an array of strings regarded as web image content types in which
variants can be processed without being converted to the fallback PNG format.
If you want to use `WebP` or `AVIF` variants in your application you can add
`image/webp` or `image/avif` to this array.
By default, this is defined as:

```ruby
config.active_storage.web_image_content_types = %w(image/png image/jpeg image/gif)
```

#### `config.active_storage.content_types_to_serve_as_binary`

Accepts an array of strings indicating the content types that Active Storage will always serve as an attachment, rather than inline.
By default, this is defined as:

```ruby
config.active_storage.content_types_to_serve_as_binary = %w(text/html image/svg+xml application/postscript application/x-shockwave-flash text/xml application/xml application/xhtml+xml application/mathml+xml text/cache-manifest)
```

#### `config.active_storage.content_types_allowed_inline`

Accepts an array of strings indicating the content types that Active Storage allows to serve as inline.
By default, this is defined as:

```ruby
config.active_storage.content_types_allowed_inline` = %w(image/png image/gif image/jpeg image/tiff image/vnd.adobe.photoshop image/vnd.microsoft.icon application/pdf)
```

#### `config.active_storage.silence_invalid_content_types_warning`

Since Rails 7, Active Storage will warn if you use an invalid content type that was incorrectly supported in Rails 6. You can use this config to turn the warning off.

```ruby
config.active_storage.silence_invalid_content_types_warning = false
```

#### `config.active_storage.queues.analysis`

Accepts a symbol indicating the Active Job queue to use for analysis jobs. When this option is `nil`, analysis jobs are sent to the default Active Job queue (see `config.active_job.default_queue_name`).

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| 6.0                   | `:active_storage_analysis` |
| 6.1                   | `nil`                |

#### `config.active_storage.queues.purge`

Accepts a symbol indicating the Active Job queue to use for purge jobs. When this option is `nil`, purge jobs are sent to the default Active Job queue (see `config.active_job.default_queue_name`).

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| 6.0                   | `:active_storage_purge` |
| 6.1                   | `nil`                |

#### `config.active_storage.queues.mirror`

Accepts a symbol indicating the Active Job queue to use for direct upload mirroring jobs. When this option is `nil`, mirroring jobs are sent to the default Active Job queue (see `config.active_job.default_queue_name`). The default is `nil`.

#### `config.active_storage.logger`

Can be used to set the logger used by Active Storage. Accepts a logger conforming to the interface of Log4r or the default Ruby Logger class.

```ruby
config.active_storage.logger = ActiveSupport::Logger.new(STDOUT)
```

#### `config.active_storage.service_urls_expire_in`

Determines the default expiry of URLs generated by:

* `ActiveStorage::Blob#url`
* `ActiveStorage::Blob#service_url_for_direct_upload`
* `ActiveStorage::Variant#url`

The default is 5 minutes.

#### `config.active_storage.urls_expire_in`

Determines the default expiry of URLs in the Rails application generated by Active Storage. The default is nil.

#### `config.active_storage.routes_prefix`

Can be used to set the route prefix for the routes served by Active Storage. Accepts a string that will be prepended to the generated routes.

```ruby
config.active_storage.routes_prefix = '/files'
```

The default is `/rails/active_storage`.

#### `config.active_storage.replace_on_assign_to_many`

Determines whether assigning to a collection of attachments declared with `has_many_attached` replaces any existing attachments or appends to them.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 6.0                   | `true`               |

#### `config.active_storage.track_variants`

Determines whether variants are recorded in the database.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 6.1                   | `true`               |

#### `config.active_storage.draw_routes`

Can be used to toggle Active Storage route generation. The default is `true`.

#### `config.active_storage.resolve_model_to_route`

Can be used to globally change how Active Storage files are delivered.

Allowed values are:

* `:rails_storage_redirect`: Redirect to signed, short-lived service URLs.
* `:rails_storage_proxy`: Proxy files by downloading them.

The default is `:rails_storage_redirect`.

#### `config.active_storage.video_preview_arguments`

Can be used to alter the way ffmpeg generates video preview images.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `"-y -vframes 1 -f image2"` |
| 7.0                   | `"-vf 'select=eq(n\\,0)+eq(key\\,1)+gt(scene\\,0.015)"`<sup><mark><strong><em>1</em></strong></mark></sup> <br> `+ ",loop=loop=-1:size=2,trim=start_frame=1'"`<sup><mark><strong><em>2</em></strong></mark></sup><br> `+ " -frames:v 1 -f image2"` <br><br> <ol><li>Select the first video frame, plus keyframes, plus frames that meet the scene change threshold.</li> <li>Use the first video frame as a fallback when no other frames meet the criteria by looping the first (one or) two selected frames, then dropping the first looped frame.</li></ol> |

#### `config.active_storage.multiple_file_field_include_hidden`

In Rails 7.1 and beyond, Active Storage `has_many_attached` relationships will
default to _replacing_ the current collection instead of _appending_ to it. Thus
to support submitting an _empty_ collection, when `multiple_file_field_include_hidden`
is `true`, the [`file_field`](https://api.rubyonrails.org/classes/ActionView/Helpers/FormBuilder.html#method-i-file_field)
helper will render an auxiliary hidden field, similar to the auxiliary field
rendered by the [`check_box`](https://api.rubyonrails.org/classes/ActionView/Helpers/FormBuilder.html#method-i-check_box)
helper.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 7.0                   | `true`               |

#### `config.active_storage.precompile_assets`

Determines whether the Active Storage assets should be added to the asset pipeline precompilation. It
has no effect if Sprockets is not used. The default value is `true`.

### Configuring Action Text

#### `config.action_text.attachment_tag_name`

Accepts a string for the HTML tag used to wrap attachments. Defaults to `"action-text-attachment"`.

### Configuring a Database

Just about every Rails application will interact with a database. You can connect to the database by setting an environment variable `ENV['DATABASE_URL']` or by using a configuration file called `config/database.yml`.

Using the `config/database.yml` file you can specify all the information needed to access your database:

```yaml
development:
  adapter: postgresql
  database: blog_development
  pool: 5
```

This will connect to the database named `blog_development` using the `postgresql` adapter. This same information can be stored in a URL and provided via an environment variable like this:

```ruby
ENV['DATABASE_URL'] # => "postgresql://localhost/blog_development?pool=5"
```

The `config/database.yml` file contains sections for three different environments in which Rails can run by default:

* The `development` environment is used on your development/local computer as you interact manually with the application.
* The `test` environment is used when running automated tests.
* The `production` environment is used when you deploy your application for the world to use.

If you wish, you can manually specify a URL inside of your `config/database.yml`

```yaml
development:
  url: postgresql://localhost/blog_development?pool=5
```

The `config/database.yml` file can contain ERB tags `<%= %>`. Anything in the tags will be evaluated as Ruby code. You can use this to pull out data from an environment variable or to perform calculations to generate the needed connection information.


TIP: You don't have to update the database configurations manually. If you look at the options of the application generator, you will see that one of the options is named `--database`. This option allows you to choose an adapter from a list of the most used relational databases. You can even run the generator repeatedly: `cd .. && rails new blog --database=mysql`. When you confirm the overwriting of the `config/database.yml` file, your application will be configured for MySQL instead of SQLite. Detailed examples of the common database connections are below.

### Connection Preference

Since there are two ways to configure your connection (using `config/database.yml` or using an environment variable) it is important to understand how they can interact.

If you have an empty `config/database.yml` file but your `ENV['DATABASE_URL']` is present, then Rails will connect to the database via your environment variable:

```bash
$ cat config/database.yml

$ echo $DATABASE_URL
postgresql://localhost/my_database
```

If you have a `config/database.yml` but no `ENV['DATABASE_URL']` then this file will be used to connect to your database:

```bash
$ cat config/database.yml
development:
  adapter: postgresql
  database: my_database
  host: localhost

$ echo $DATABASE_URL
```

If you have both `config/database.yml` and `ENV['DATABASE_URL']` set then Rails will merge the configuration together. To better understand this we must see some examples.

When duplicate connection information is provided the environment variable will take precedence:

```bash
$ cat config/database.yml
development:
  adapter: sqlite3
  database: NOT_my_database
  host: localhost

$ echo $DATABASE_URL
postgresql://localhost/my_database

$ bin/rails runner 'puts ActiveRecord::Base.configurations'
#<ActiveRecord::DatabaseConfigurations:0x00007fd50e209a28>

$ bin/rails runner 'puts ActiveRecord::Base.configurations.inspect'
#<ActiveRecord::DatabaseConfigurations:0x00007fc8eab02880 @configurations=[
  #<ActiveRecord::DatabaseConfigurations::UrlConfig:0x00007fc8eab020b0
    @env_name="development", @spec_name="primary",
    @config={"adapter"=>"postgresql", "database"=>"my_database", "host"=>"localhost"}
    @url="postgresql://localhost/my_database">
  ]
```

Here the adapter, host, and database match the information in `ENV['DATABASE_URL']`.

If non-duplicate information is provided you will get all unique values, environment variable still takes precedence in cases of any conflicts.

```bash
$ cat config/database.yml
development:
  adapter: sqlite3
  pool: 5

$ echo $DATABASE_URL
postgresql://localhost/my_database

$ bin/rails runner 'puts ActiveRecord::Base.configurations'
#<ActiveRecord::DatabaseConfigurations:0x00007fd50e209a28>

$ bin/rails runner 'puts ActiveRecord::Base.configurations.inspect'
#<ActiveRecord::DatabaseConfigurations:0x00007fc8eab02880 @configurations=[
  #<ActiveRecord::DatabaseConfigurations::UrlConfig:0x00007fc8eab020b0
    @env_name="development", @spec_name="primary",
    @config={"adapter"=>"postgresql", "database"=>"my_database", "host"=>"localhost", "pool"=>5}
    @url="postgresql://localhost/my_database">
  ]
```

Since pool is not in the `ENV['DATABASE_URL']` provided connection information its information is merged in. Since `adapter` is duplicate, the `ENV['DATABASE_URL']` connection information wins.

The only way to explicitly not use the connection information in `ENV['DATABASE_URL']` is to specify an explicit URL connection using the `"url"` sub key:

```bash
$ cat config/database.yml
development:
  url: sqlite3:NOT_my_database

$ echo $DATABASE_URL
postgresql://localhost/my_database

$ bin/rails runner 'puts ActiveRecord::Base.configurations'
#<ActiveRecord::DatabaseConfigurations:0x00007fd50e209a28>

$ bin/rails runner 'puts ActiveRecord::Base.configurations.inspect'
#<ActiveRecord::DatabaseConfigurations:0x00007fc8eab02880 @configurations=[
  #<ActiveRecord::DatabaseConfigurations::UrlConfig:0x00007fc8eab020b0
    @env_name="development", @spec_name="primary",
    @config={"adapter"=>"sqlite3", "database"=>"NOT_my_database"}
    @url="sqlite3:NOT_my_database">
  ]
```

Here the connection information in `ENV['DATABASE_URL']` is ignored, note the different adapter and database name.

Since it is possible to embed ERB in your `config/database.yml` it is best practice to explicitly show you are using the `ENV['DATABASE_URL']` to connect to your database. This is especially useful in production since you should not commit secrets like your database password into your source control (such as Git).

```bash
$ cat config/database.yml
production:
  url: <%= ENV['DATABASE_URL'] %>
```

Now the behavior is clear, that we are only using the connection information in `ENV['DATABASE_URL']`.

#### Configuring an SQLite3 Database

Rails comes with built-in support for [SQLite3](http://www.sqlite.org), which is a lightweight serverless database application. While a busy production environment may overload SQLite, it works well for development and testing. Rails defaults to using an SQLite database when creating a new project, but you can always change it later.

Here's the section of the default configuration file (`config/database.yml`) with connection information for the development environment:

```yaml
development:
  adapter: sqlite3
  database: db/development.sqlite3
  pool: 5
  timeout: 5000
```

NOTE: Rails uses an SQLite3 database for data storage by default because it is a zero configuration database that just works. Rails also supports MySQL (including MariaDB) and PostgreSQL "out of the box", and has plugins for many database systems. If you are using a database in a production environment Rails most likely has an adapter for it.

#### Configuring a MySQL or MariaDB Database

If you choose to use MySQL or MariaDB instead of the shipped SQLite3 database, your `config/database.yml` will look a little different. Here's the development section:

```yaml
development:
  adapter: mysql2
  encoding: utf8mb4
  database: blog_development
  pool: 5
  username: root
  password:
  socket: /tmp/mysql.sock
```

If your development database has a root user with an empty password, this configuration should work for you. Otherwise, change the username and password in the `development` section as appropriate.

NOTE: If your MySQL version is 5.5 or 5.6 and want to use the `utf8mb4` character set by default, please configure your MySQL server to support the longer key prefix by enabling `innodb_large_prefix` system variable.

Advisory Locks are enabled by default on MySQL and are used to make database migrations concurrent safe. You can disable advisory locks by setting `advisory_locks` to `false`:

```yaml
production:
  adapter: mysql2
  advisory_locks: false
```

#### Configuring a PostgreSQL Database

If you choose to use PostgreSQL, your `config/database.yml` will be customized to use PostgreSQL databases:

```yaml
development:
  adapter: postgresql
  encoding: unicode
  database: blog_development
  pool: 5
```

By default Active Record uses database features like prepared statements and advisory locks. You might need to disable those features if you're using an external connection pooler like PgBouncer:

```yaml
production:
  adapter: postgresql
  prepared_statements: false
  advisory_locks: false
```

If enabled, Active Record will create up to `1000` prepared statements per database connection by default. To modify this behavior you can set `statement_limit` to a different value:

```yaml
production:
  adapter: postgresql
  statement_limit: 200
```

The more prepared statements in use: the more memory your database will require. If your PostgreSQL database is hitting memory limits, try lowering `statement_limit` or disabling prepared statements.

#### Configuring an SQLite3 Database for JRuby Platform

If you choose to use SQLite3 and are using JRuby, your `config/database.yml` will look a little different. Here's the development section:

```yaml
development:
  adapter: jdbcsqlite3
  database: db/development.sqlite3
```

#### Configuring a MySQL or MariaDB Database for JRuby Platform

If you choose to use MySQL or MariaDB and are using JRuby, your `config/database.yml` will look a little different. Here's the development section:

```yaml
development:
  adapter: jdbcmysql
  database: blog_development
  username: root
  password:
```

#### Configuring a PostgreSQL Database for JRuby Platform

If you choose to use PostgreSQL and are using JRuby, your `config/database.yml` will look a little different. Here's the development section:

```yaml
development:
  adapter: jdbcpostgresql
  encoding: unicode
  database: blog_development
  username: blog
  password:
```

Change the username and password in the `development` section as appropriate.

#### Configuring Metadata Storage

By default Rails will store information about your Rails environment and schema
in an internal table named `ar_internal_metadata`.

To turn this off per connection, set `use_metadata_table` in your database
configuration. This is useful when working with a shared database and/or
database user that cannot create tables.

```yaml
development:
  adapter: postgresql
  use_metadata_table: false
```

### Creating Rails Environments

By default Rails ships with three environments: "development", "test", and "production". While these are sufficient for most use cases, there are circumstances when you want more environments.

Imagine you have a server which mirrors the production environment but is only used for testing. Such a server is commonly called a "staging server". To define an environment called "staging" for this server, just create a file called `config/environments/staging.rb`. Please use the contents of any existing file in `config/environments` as a starting point and make the necessary changes from there.

That environment is no different than the default ones, start a server with `bin/rails server -e staging`, a console with `bin/rails console -e staging`, `Rails.env.staging?` works, etc.

### Deploy to a Subdirectory (relative URL root)

By default Rails expects that your application is running at the root
(e.g. `/`). This section explains how to run your application inside a directory.

Let's assume we want to deploy our application to "/app1". Rails needs to know
this directory to generate the appropriate routes:

```ruby
config.relative_url_root = "/app1"
```

alternatively you can set the `RAILS_RELATIVE_URL_ROOT` environment
variable.

Rails will now prepend "/app1" when generating links.

#### Using Passenger

Passenger makes it easy to run your application in a subdirectory. You can find the relevant configuration in the [Passenger manual](https://www.phusionpassenger.com/library/deploy/apache/deploy/ruby/#deploying-an-app-to-a-sub-uri-or-subdirectory).

#### Using a Reverse Proxy

Deploying your application using a reverse proxy has definite advantages over traditional deploys. They allow you to have more control over your server by layering the components required by your application.

Many modern web servers can be used as a proxy server to balance third-party elements such as caching servers or application servers.

One such application server you can use is [Unicorn](https://bogomips.org/unicorn/) to run behind a reverse proxy.

In this case, you would need to configure the proxy server (NGINX, Apache, etc) to accept connections from your application server (Unicorn). By default Unicorn will listen for TCP connections on port 8080, but you can change the port or configure it to use sockets instead.

You can find more information in the [Unicorn readme](https://bogomips.org/unicorn/README.html) and understand the [philosophy](https://bogomips.org/unicorn/PHILOSOPHY.html) behind it.

Once you've configured the application server, you must proxy requests to it by configuring your web server appropriately. For example your NGINX config may include:

```nginx
upstream application_server {
  server 0.0.0.0:8080;
}

server {
  listen 80;
  server_name localhost;

  root /root/path/to/your_app/public;

  try_files $uri/index.html $uri.html @app;

  location @app {
    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
    proxy_set_header Host $http_host;
    proxy_redirect off;
    proxy_pass http://application_server;
  }

  # some other configuration
}
```

Be sure to read the [NGINX documentation](https://nginx.org/en/docs/) for the most up-to-date information.


Rails Environment Settings
--------------------------

Some parts of Rails can also be configured externally by supplying environment variables. The following environment variables are recognized by various parts of Rails:

* `ENV["RAILS_ENV"]` defines the Rails environment (production, development, test, and so on) that Rails will run under.

* `ENV["RAILS_RELATIVE_URL_ROOT"]` is used by the routing code to recognize URLs when you [deploy your application to a subdirectory](configuring.html#deploy-to-a-subdirectory-relative-url-root).

* `ENV["RAILS_CACHE_ID"]` and `ENV["RAILS_APP_VERSION"]` are used to generate expanded cache keys in Rails' caching code. This allows you to have multiple separate caches from the same application.


Using Initializer Files
-----------------------

After loading the framework and any gems in your application, Rails turns to
loading initializers. An initializer is any Ruby file stored under
`config/initializers` in your application. You can use initializers to hold
configuration settings that should be made after all of the frameworks and gems
are loaded, such as options to configure settings for these parts.

The files in `config/initializers` (and any subdirectories of
`config/initializers`) are sorted and loaded one by one as part of
the `load_config_initializers` initializer.

If an initializer has code that relies on code in another initializer, you can
combine them into a single initializer instead. This makes the dependencies more
explicit, and can help surface new concepts within your application. Rails also
supports numbering of initializer file names, but this can lead to file name
churn. Explicitly loading initializers with `require` is not recommended, since
it will cause the initializer to get loaded twice.

NOTE: There is no guarantee that your initializers will run after all the gem
initializers, so any initialization code that depends on a given gem having been
initialized should go into a `config.after_initialize` block.

Initialization events
---------------------

Rails has 5 initialization events which can be hooked into (listed in the order that they are run):

* `before_configuration`: This is run as soon as the application constant inherits from `Rails::Application`. The `config` calls are evaluated before this happens.

* `before_initialize`: This is run directly before the initialization process of the application occurs with the `:bootstrap_hook` initializer near the beginning of the Rails initialization process.

* `to_prepare`: Run after the initializers are run for all Railties (including the application itself), but before eager loading and the middleware stack is built. More importantly, will run upon every code reload in `development`, but only once (during boot-up) in `production` and `test`.

* `before_eager_load`: This is run directly before eager loading occurs, which is the default behavior for the `production` environment and not for the `development` environment.

* `after_initialize`: Run directly after the initialization of the application, after the application initializers in `config/initializers` are run.

To define an event for these hooks, use the block syntax within a `Rails::Application`, `Rails::Railtie` or `Rails::Engine` subclass:

```ruby
module YourApp
  class Application < Rails::Application
    config.before_initialize do
      # initialization code goes here
    end
  end
end
```

Alternatively, you can also do it through the `config` method on the `Rails.application` object:

```ruby
Rails.application.config.before_initialize do
  # initialization code goes here
end
```

WARNING: Some parts of your application, notably routing, are not yet set up at the point where the `after_initialize` block is called.

### `Rails::Railtie#initializer`

Rails has several initializers that run on startup that are all defined by using the `initializer` method from `Rails::Railtie`. Here's an example of the `set_helpers_path` initializer from Action Controller:

```ruby
initializer "action_controller.set_helpers_path" do |app|
  ActionController::Helpers.helpers_path = app.helpers_paths
end
```

The `initializer` method takes three arguments with the first being the name for the initializer and the second being an options hash (not shown here) and the third being a block. The `:before` key in the options hash can be specified to specify which initializer this new initializer must run before, and the `:after` key will specify which initializer to run this initializer _after_.

Initializers defined using the `initializer` method will be run in the order they are defined in, with the exception of ones that use the `:before` or `:after` methods.

WARNING: You may put your initializer before or after any other initializer in the chain, as long as it is logical. Say you have 4 initializers called "one" through "four" (defined in that order) and you define "four" to go _before_ "two" but _after_ "three", that just isn't logical and Rails will not be able to determine your initializer order.

The block argument of the `initializer` method is the instance of the application itself, and so we can access the configuration on it by using the `config` method as done in the example.

Because `Rails::Application` inherits from `Rails::Railtie` (indirectly), you can use the `initializer` method in `config/application.rb` to define initializers for the application.

### Initializers

Below is a comprehensive list of all the initializers found in Rails in the order that they are defined (and therefore run in, unless otherwise stated).

* `load_environment_hook`: Serves as a placeholder so that `:load_environment_config` can be defined to run before it.

* `load_active_support`: Requires `active_support/dependencies` which sets up the basis for Active Support. Optionally requires `active_support/all` if `config.active_support.bare` is un-truthful, which is the default.

* `initialize_logger`: Initializes the logger (an `ActiveSupport::Logger` object) for the application and makes it accessible at `Rails.logger`, provided that no initializer inserted before this point has defined `Rails.logger`.

* `initialize_cache`: If `Rails.cache` isn't set yet, initializes the cache by referencing the value in `config.cache_store` and stores the outcome as `Rails.cache`. If this object responds to the `middleware` method, its middleware is inserted before `Rack::Runtime` in the middleware stack.

* `set_clear_dependencies_hook`: This initializer - which runs only if `config.enable_reloading` is set to `true` - uses `ActionDispatch::Callbacks.after` to remove the constants which have been referenced during the request from the object space so that they will be reloaded during the following request.

* `bootstrap_hook`: Runs all configured `before_initialize` blocks.

* `i18n.callbacks`: In the development environment, sets up a `to_prepare` callback which will call `I18n.reload!` if any of the locales have changed since the last request. In production this callback will only run on the first request.

* `active_support.deprecation_behavior`: Sets up deprecation reporting for environments, defaulting to `:log` for development, `:silence` for production, and `:stderr` for test.  Can be set to an array of values. This initializer also sets up behaviors for disallowed deprecations, defaulting to `:raise` for development and test and `:silence` for production. Disallowed deprecation warnings default to an empty array.

* `active_support.initialize_time_zone`: Sets the default time zone for the application based on the `config.time_zone` setting, which defaults to "UTC".

* `active_support.initialize_beginning_of_week`: Sets the default beginning of week for the application based on `config.beginning_of_week` setting, which defaults to `:monday`.

* `active_support.set_configs`: Sets up Active Support by using the settings in `config.active_support` by `send`'ing the method names as setters to `ActiveSupport` and passing the values through.

* `action_dispatch.configure`: Configures the `ActionDispatch::Http::URL.tld_length` to be set to the value of `config.action_dispatch.tld_length`.

* `action_view.set_configs`: Sets up Action View by using the settings in `config.action_view` by `send`'ing the method names as setters to `ActionView::Base` and passing the values through.

* `action_controller.assets_config`: Initializes the `config.action_controller.assets_dir` to the app's public directory if not explicitly configured.

* `action_controller.set_helpers_path`: Sets Action Controller's `helpers_path` to the application's `helpers_path`.

* `action_controller.parameters_config`: Configures strong parameters options for `ActionController::Parameters`.

* `action_controller.set_configs`: Sets up Action Controller by using the settings in `config.action_controller` by `send`'ing the method names as setters to `ActionController::Base` and passing the values through.

* `action_controller.compile_config_methods`: Initializes methods for the config settings specified so that they are quicker to access.

* `active_record.initialize_timezone`: Sets `ActiveRecord::Base.time_zone_aware_attributes` to `true`, as well as setting `ActiveRecord::Base.default_timezone` to UTC. When attributes are read from the database, they will be converted into the time zone specified by `Time.zone`.

* `active_record.logger`: Sets `ActiveRecord::Base.logger` - if it's not already set - to `Rails.logger`.

* `active_record.migration_error`: Configures middleware to check for pending migrations.

* `active_record.check_schema_cache_dump`: Loads the schema cache dump if configured and available.

* `active_record.warn_on_records_fetched_greater_than`: Enables warnings when queries return large numbers of records.

* `active_record.set_configs`: Sets up Active Record by using the settings in `config.active_record` by `send`'ing the method names as setters to `ActiveRecord::Base` and passing the values through.

* `active_record.initialize_database`: Loads the database configuration (by default) from `config/database.yml` and establishes a connection for the current environment.

* `active_record.log_runtime`: Includes `ActiveRecord::Railties::ControllerRuntime` and `ActiveRecord::Railties::JobRuntime` which are responsible for reporting the time taken by Active Record calls for the request back to the logger.

* `active_record.set_reloader_hooks`: Resets all reloadable connections to the database if `config.enable_reloading` is set to `true`.

* `active_record.add_watchable_files`: Adds `schema.rb` and `structure.sql` files to watchable files.

* `active_job.logger`: Sets `ActiveJob::Base.logger` - if it's not already set -
  to `Rails.logger`.

* `active_job.set_configs`: Sets up Active Job by using the settings in `config.active_job` by `send`'ing the method names as setters to `ActiveJob::Base` and passing the values through.

* `action_mailer.logger`: Sets `ActionMailer::Base.logger` - if it's not already set - to `Rails.logger`.

* `action_mailer.set_configs`: Sets up Action Mailer by using the settings in `config.action_mailer` by `send`'ing the method names as setters to `ActionMailer::Base` and passing the values through.

* `action_mailer.compile_config_methods`: Initializes methods for the config settings specified so that they are quicker to access.

* `set_load_path`: This initializer runs before `bootstrap_hook`. Adds paths specified by `config.load_paths` and all autoload paths to `$LOAD_PATH`.

* `set_autoload_paths`: This initializer runs before `bootstrap_hook`. Adds all sub-directories of `app` and paths specified by `config.autoload_paths`, `config.eager_load_paths` and `config.autoload_once_paths` to `ActiveSupport::Dependencies.autoload_paths`.

* `add_routing_paths`: Loads (by default) all `config/routes.rb` files (in the application and railties, including engines) and sets up the routes for the application.

* `add_locales`: Adds the files in `config/locales` (from the application, railties, and engines) to `I18n.load_path`, making available the translations in these files.

* `add_view_paths`: Adds the directory `app/views` from the application, railties, and engines to the lookup path for view files for the application.

* `load_environment_config`: Loads the `config/environments` file for the current environment.

* `prepend_helpers_path`: Adds the directory `app/helpers` from the application, railties, and engines to the lookup path for helpers for the application.

* `load_config_initializers`: Loads all Ruby files from `config/initializers` in the application, railties, and engines. The files in this directory can be used to hold configuration settings that should be made after all of the frameworks are loaded.

* `engines_blank_point`: Provides a point-in-initialization to hook into if you wish to do anything before engines are loaded. After this point, all railtie and engine initializers are run.

* `add_generator_templates`: Finds templates for generators at `lib/templates` for the application, railties, and engines, and adds these to the `config.generators.templates` setting, which will make the templates available for all generators to reference.

* `ensure_autoload_once_paths_as_subset`: Ensures that the `config.autoload_once_paths` only contains paths from `config.autoload_paths`. If it contains extra paths, then an exception will be raised.

* `add_to_prepare_blocks`: The block for every `config.to_prepare` call in the application, a railtie, or engine is added to the `to_prepare` callbacks for Action Dispatch which will be run per request in development, or before the first request in production.

* `add_builtin_route`: If the application is running under the development environment then this will append the route for `rails/info/properties` to the application routes. This route provides the detailed information such as Rails and Ruby version for `public/index.html` in a default Rails application.

* `build_middleware_stack`: Builds the middleware stack for the application, returning an object which has a `call` method which takes a Rack environment object for the request.

* `eager_load!`: If `config.eager_load` is `true`, runs the `config.before_eager_load` hooks and then calls `eager_load!` which will load all `config.eager_load_namespaces`.

* `finisher_hook`: Provides a hook for after the initialization of process of the application is complete, as well as running all the `config.after_initialize` blocks for the application, railties, and engines.

* `set_routes_reloader_hook`: Configures Action Dispatch to reload the routes file using `ActiveSupport::Callbacks.to_run`.

* `disable_dependency_loading`: Disables the automatic dependency loading if the `config.eager_load` is set to `true`.

Database pooling
----------------

Active Record database connections are managed by `ActiveRecord::ConnectionAdapters::ConnectionPool` which ensures that a connection pool synchronizes the amount of thread access to a limited number of database connections. This limit defaults to 5 and can be configured in `database.yml`.

```ruby
development:
  adapter: sqlite3
  database: db/development.sqlite3
  pool: 5
  timeout: 5000
```

Since the connection pooling is handled inside of Active Record by default, all application servers (Thin, Puma, Unicorn, etc.) should behave the same. The database connection pool is initially empty. As demand for connections increases it will create them until it reaches the connection pool limit.

Any one request will check out a connection the first time it requires access to the database. At the end of the request it will check the connection back in. This means that the additional connection slot will be available again for the next request in the queue.

If you try to use more connections than are available, Active Record will block
you and wait for a connection from the pool. If it cannot get a connection, a
timeout error similar to that given below will be thrown.

```ruby
ActiveRecord::ConnectionTimeoutError - could not obtain a database connection within 5.000 seconds (waited 5.000 seconds)
```

If you get the above error, you might want to increase the size of the
connection pool by incrementing the `pool` option in `database.yml`

NOTE. If you are running in a multi-threaded environment, there could be a chance that several threads may be accessing multiple connections simultaneously. So depending on your current request load, you could very well have multiple threads contending for a limited number of connections.


Custom configuration
--------------------

You can configure your own code through the Rails configuration object with
custom configuration under either the `config.x` namespace, or `config` directly.
The key difference between these two is that you should be using `config.x` if you
are defining _nested_ configuration (ex: `config.x.nested.hi`), and just
`config` for _single level_ configuration (ex: `config.hello`).

```ruby
config.x.payment_processing.schedule = :daily
config.x.payment_processing.retries  = 3
config.super_debugger = true
```

These configuration points are then available through the configuration object:

```ruby
Rails.configuration.x.payment_processing.schedule # => :daily
Rails.configuration.x.payment_processing.retries  # => 3
Rails.configuration.x.payment_processing.not_set  # => nil
Rails.configuration.super_debugger                # => true
```

You can also use `Rails::Application.config_for` to load whole configuration files:

```yaml
# config/payment.yml
production:
  environment: production
  merchant_id: production_merchant_id
  public_key:  production_public_key
  private_key: production_private_key

development:
  environment: sandbox
  merchant_id: development_merchant_id
  public_key:  development_public_key
  private_key: development_private_key
```

```ruby
# config/application.rb
module MyApp
  class Application < Rails::Application
    config.payment = config_for(:payment)
  end
end
```

```ruby
Rails.configuration.payment['merchant_id'] # => production_merchant_id or development_merchant_id
```

`Rails::Application.config_for` supports a `shared` configuration to group common
configurations. The shared configuration will be merged into the environment
configuration.

```yaml
# config/example.yml
shared:
  foo:
    bar:
      baz: 1

development:
  foo:
    bar:
      qux: 2
```

```ruby
# development environment
Rails.application.config_for(:example)[:foo][:bar] #=> { baz: 1, qux: 2 }
```

Search Engines Indexing
-----------------------

Sometimes, you may want to prevent some pages of your application to be visible
on search sites like Google, Bing, Yahoo, or Duck Duck Go. The robots that index
these sites will first analyze the `http://your-site.com/robots.txt` file to
know which pages it is allowed to index.

Rails creates this file for you inside the `/public` folder. By default, it allows
search engines to index all pages of your application. If you want to block
indexing on all pages of your application, use this:

```
User-agent: *
Disallow: /
```

To block just specific pages, it's necessary to use a more complex syntax. Learn
it on the [official documentation](https://www.robotstxt.org/robotstxt.html).

Evented File System Monitor
---------------------------

If the [listen gem](https://github.com/guard/listen) is loaded Rails uses an
evented file system monitor to detect changes when reloading is enabled:

```ruby
group :development do
  gem 'listen', '~> 3.3'
end
```

Otherwise, in every request Rails walks the application tree to check if
anything has changed.

On Linux and macOS no additional gems are needed, but some are required
[for *BSD](https://github.com/guard/listen#on-bsd) and
[for Windows](https://github.com/guard/listen#on-windows).

Note that [some setups are unsupported](https://github.com/guard/listen#issues--limitations).