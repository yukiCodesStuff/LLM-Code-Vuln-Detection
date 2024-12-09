=> {
  :name => ["can't be blank"],
  :age  => ["Please fill in your Age"]
}
```

[ActiveModel::Error#full_message]: https://api.rubyonrails.org/classes/ActiveModel/Error.html#method-i-full_message

### Configuring Active Record

`config.active_record` includes a variety of configuration options:

#### `config.active_record.logger`

Accepts a logger conforming to the interface of Log4r or the default Ruby Logger class, which is then passed on to any new database connections made. You can retrieve this logger by calling `logger` on either an Active Record model class or an Active Record model instance. Set to `nil` to disable logging.

#### `config.active_record.primary_key_prefix_type`

Lets you adjust the naming for primary key columns. By default, Rails assumes that primary key columns are named `id` (and this configuration option doesn't need to be set). There are two other choices:

* `:table_name` would make the primary key for the Customer class `customerid`.
* `:table_name_with_underscore` would make the primary key for the Customer class `customer_id`.

#### `config.active_record.table_name_prefix`

Lets you set a global string to be prepended to table names. If you set this to `northwest_`, then the Customer class will look for `northwest_customers` as its table. The default is an empty string.

#### `config.active_record.table_name_suffix`

Lets you set a global string to be appended to table names. If you set this to `_northwest`, then the Customer class will look for `customers_northwest` as its table. The default is an empty string.

#### `config.active_record.schema_migrations_table_name`

Lets you set a string to be used as the name of the schema migrations table.

#### `config.active_record.internal_metadata_table_name`

Lets you set a string to be used as the name of the internal metadata table.

#### `config.active_record.protected_environments`

Lets you set an array of names of environments where destructive actions should be prohibited.

#### `config.active_record.pluralize_table_names`

Specifies whether Rails will look for singular or plural table names in the database. If set to `true` (the default), then the Customer class will use the `customers` table. If set to `false`, then the Customer class will use the `customer` table.

#### `config.active_record.default_timezone`

Determines whether to use `Time.local` (if set to `:local`) or `Time.utc` (if set to `:utc`) when pulling dates and times from the database. The default is `:utc`.

#### `config.active_record.schema_format`

Controls the format for dumping the database schema to a file. The options are `:ruby` (the default) for a database-independent version that depends on migrations, or `:sql` for a set of (potentially database-dependent) SQL statements.

#### `config.active_record.error_on_ignored_order`

Specifies if an error should be raised if the order of a query is ignored during a batch query. The options are `true` (raise error) or `false` (warn). Default is `false`.

#### `config.active_record.timestamped_migrations`

Controls whether migrations are numbered with serial integers or with timestamps. The default is `true`, to use timestamps, which are preferred if there are multiple developers working on the same application.

#### `config.active_record.migration_strategy`

Controls the strategy class used to perform schema statement methods in a migration. The default class
delegates to the connection adapter. Custom strategies should inherit from `ActiveRecord::Migration::ExecutionStrategy`,
or may inherit from `DefaultStrategy`, which will preserve the default behaviour for methods that aren't implemented:

```ruby
class CustomMigrationStrategy < ActiveRecord::Migration::DefaultStrategy
  def drop_table(*)
    raise "Dropping tables is not supported!"
  end
end

config.active_record.migration_strategy = CustomMigrationStrategy
```

#### `config.active_record.lock_optimistically`

Controls whether Active Record will use optimistic locking and is `true` by default.

#### `config.active_record.cache_timestamp_format`

Controls the format of the timestamp value in the cache key. Default is `:usec`.

#### `config.active_record.record_timestamps`

Is a boolean value which controls whether or not timestamping of `create` and `update` operations on a model occur. The default value is `true`.

#### `config.active_record.partial_inserts`

Is a boolean value and controls whether or not partial writes are used when creating new records (i.e. whether inserts only set attributes that are different from the default).

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `true`               |
| 7.0                   | `false`              |

#### `config.active_record.partial_updates`

Is a boolean value and controls whether or not partial writes are used when updating existing records (i.e. whether updates only set attributes that are dirty). Note that when using partial updates, you should also use optimistic locking `config.active_record.lock_optimistically` since concurrent updates may write attributes based on a possibly stale read state. The default value is `true`.

#### `config.active_record.maintain_test_schema`

Is a boolean value which controls whether Active Record should try to keep your test database schema up-to-date with `db/schema.rb` (or `db/structure.sql`) when you run your tests. The default is `true`.

#### `config.active_record.dump_schema_after_migration`

Is a flag which controls whether or not schema dump should happen
(`db/schema.rb` or `db/structure.sql`) when you run migrations. This is set to
`false` in `config/environments/production.rb` which is generated by Rails. The
default value is `true` if this configuration is not set.

#### `config.active_record.dump_schemas`

Controls which database schemas will be dumped when calling `db:schema:dump`.
The options are `:schema_search_path` (the default) which dumps any schemas listed in `schema_search_path`,
`:all` which always dumps all schemas regardless of the `schema_search_path`,
or a string of comma separated schemas.

#### `config.active_record.before_committed_on_all_records`

Enable before_committed! callbacks on all enrolled records in a transaction.
The previous behavior was to only run the callbacks on the first copy of a record
if there were multiple copies of the same record enrolled in the transaction.

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 7.1                   | `true`               |

#### `config.active_record.belongs_to_required_by_default`

Is a boolean value and controls whether a record fails validation if
`belongs_to` association is not present.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `nil`                |
| 5.0                   | `true`               |

#### `config.active_record.belongs_to_required_validates_foreign_key`

Enable validating only parent-related columns for presence when the parent is mandatory.
The previous behavior was to validate the presence of the parent record, which performed an extra query
to get the parent every time the child record was updated, even when parent has not changed.

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `true`               |
| 7.1                   | `false`              |

#### `config.active_record.action_on_strict_loading_violation`

Enables raising or logging an exception if strict_loading is set on an
association. The default value is `:raise` in all environments. It can be
changed to `:log` to send violations to the logger instead of raising.

#### `config.active_record.strict_loading_by_default`

Is a boolean value that either enables or disables strict_loading mode by
default. Defaults to `false`.

#### `config.active_record.warn_on_records_fetched_greater_than`

Allows setting a warning threshold for query result size. If the number of
records returned by a query exceeds the threshold, a warning is logged. This
can be used to identify queries which might be causing a memory bloat.

#### `config.active_record.index_nested_attribute_errors`

Allows errors for nested `has_many` relationships to be displayed with an index
as well as the error. Defaults to `false`.

#### `config.active_record.use_schema_cache_dump`

Enables users to get schema cache information from `db/schema_cache.yml`
(generated by `bin/rails db:schema:cache:dump`), instead of having to send a
query to the database to get this information. Defaults to `true`.

#### `config.active_record.cache_versioning`

Indicates whether to use a stable `#cache_key` method that is accompanied by a
changing version in the `#cache_version` method.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 5.2                   | `true`               |

#### `config.active_record.collection_cache_versioning`

Enables the same cache key to be reused when the object being cached of type
`ActiveRecord::Relation` changes by moving the volatile information (max
updated at and count) of the relation's cache key into the cache version to
support recycling cache key.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 6.0                   | `true`               |

#### `config.active_record.has_many_inversing`

Enables setting the inverse record when traversing `belongs_to` to `has_many`
associations.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 6.1                   | `true`               |

#### `config.active_record.automatic_scope_inversing`

Enables automatically inferring the `inverse_of` for associations with a scope.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 7.0                   | `true`               |

#### `config.active_record.destroy_association_async_job`

Allows specifying the job that will be used to destroy the associated records in background. It defaults to `ActiveRecord::DestroyAssociationAsyncJob`.

#### `config.active_record.destroy_association_async_batch_size`

Allows specifying the maximum number of records that will be destroyed in a background job by the `dependent: :destroy_async` association option. All else equal, a lower batch size will enqueue more, shorter-running background jobs, while a higher batch size will enqueue fewer, longer-running background jobs. This option defaults to `nil`, which will cause all dependent records for a given association to be destroyed in the same background job.

#### `config.active_record.queues.destroy`

Allows specifying the Active Job queue to use for destroy jobs. When this option is `nil`, purge jobs are sent to the default Active Job queue (see `config.active_job.default_queue_name`). It defaults to `nil`.

#### `config.active_record.enumerate_columns_in_select_statements`

When `true`, will always include column names in `SELECT` statements, and avoid wildcard `SELECT * FROM ...` queries. This avoids prepared statement cache errors when adding columns to a PostgreSQL database for example. Defaults to `false`.

#### `config.active_record.verify_foreign_keys_for_fixtures`

Ensures all foreign key constraints are valid after fixtures are loaded in tests. Supported by PostgreSQL and SQLite only.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 7.0                   | `true`               |

#### `config.active_record.raise_on_assign_to_attr_readonly`

Enable raising on assignment to attr_readonly attributes. The previous
behavior would allow assignment but silently not persist changes to the
database.

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 7.1                   | `true`               |

#### `config.active_record.run_commit_callbacks_on_first_saved_instances_in_transaction`

When multiple Active Record instances change the same record within a transaction, Rails runs `after_commit` or `after_rollback` callbacks for only one of them. This option specifies how Rails chooses which instance receives the callbacks.

When `true`, transactional callbacks are run on the first instance to save, even though its instance state may be stale.

When `false`, transactional callbacks are run on the instances with the freshest instance state. Those instances are chosen as follows:

- In general, run transactional callbacks on the last instance to save a given record within the transaction.
- There are two exceptions:
    - If the record is created within the transaction, then updated by another instance, `after_create_commit` callbacks will be run on the second instance. This is instead of the `after_update_commit` callbacks that would naively be run based on that instance’s state.
    - If the record is destroyed within the transaction, then `after_destroy_commit` callbacks will be fired on the last destroyed instance, even if a stale instance subsequently performed an update (which will have affected 0 rows).

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `true`               |
| 7.1                   | `false`              |

#### `config.active_record.query_log_tags_enabled`

Specifies whether or not to enable adapter-level query comments. Defaults to
`false`.

#### `config.active_record.query_log_tags`

Define an `Array` specifying the key/value tags to be inserted in an SQL
comment. Defaults to `[ :application ]`, a predefined tag returning the
application name.

#### `config.active_record.query_log_tags_format`

A `Symbol` specifying the formatter to use for tags. Valid values are `:sqlcommenter` and `:legacy`.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `:legacy`            |
| 7.1                   | `:sqlcommenter`      |

#### `config.active_record.cache_query_log_tags`

Specifies whether or not to enable caching of query log tags. For applications
that have a large number of queries, caching query log tags can provide a
performance benefit when the context does not change during the lifetime of the
request or job execution. Defaults to `false`.

#### `config.active_record.schema_cache_ignored_tables`

Define the list of table that should be ignored when generating the schema
cache. It accepts an `Array` of strings, representing the table names, or
regular expressions.

#### `config.active_record.verbose_query_logs`

Specifies if source locations of methods that call database queries should be logged below relevant queries. By default, the flag is `true` in development and `false` in all other environments.

#### `config.active_record.sqlite3_adapter_strict_strings_by_default`

Specifies whether the SQLite3Adapter should be used in a strict strings mode.
The use of a strict strings mode disables double-quoted string literals.

SQLite has some quirks around double-quoted string literals.
It first tries to consider double-quoted strings as identifier names, but if they don't exist
it then considers them as string literals. Because of this, typos can silently go unnoticed.
For example, it is possible to create an index for a non existing column.
See [SQLite documentation](https://www.sqlite.org/quirks.html#double_quoted_string_literals_are_accepted) for more details.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 7.1                   | `true`               |

#### `config.active_record.async_query_executor`

Specifies how asynchronous queries are pooled.

It defaults to `nil`, which means `load_async` is disabled and instead directly executes queries in the foreground.
For queries to actually be performed asynchronously, it must be set to either `:global_thread_pool` or `:multi_thread_pool`.

`:global_thread_pool` will use a single pool for all databases the application connects to. This is the preferred configuration
for applications with only a single database, or applications which only ever query one database shard at a time.

`:multi_thread_pool` will use one pool per database, and each pool size can be configured individually in `database.yml` through the
`max_threads` and `min_thread` properties. This can be useful to applications regularly querying multiple databases at a time, and that need to more precisely define the max concurrency.

#### `config.active_record.global_executor_concurrency`

Used in conjunction with `config.active_record.async_query_executor = :global_thread_pool`, defines how many asynchronous
queries can be executed concurrently.

Defaults to `4`.

This number must be considered in accordance with the database pool size configured in `database.yml`. The connection pool
should be large enough to accommodate both the foreground threads (.e.g web server or job worker threads) and background threads.

#### `config.active_record.allow_deprecated_singular_associations_name`

This enables deprecated behavior wherein singular associations can be referred to by their plural name in `where` clauses. Setting this to `false` is more performant.

```ruby
class Comment < ActiveRecord::Base
  belongs_to :post
end

Comment.where(post: post_id).count  # => 5

# When `allow_deprecated_singular_associations_name` is true:
Comment.where(posts: post_id).count # => 5 (deprecation warning)

# When `allow_deprecated_singular_associations_name` is false:
Comment.where(posts: post_id).count # => error
```

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `true`               |
| 7.1                   | `false`              |

#### `config.active_record.yaml_column_permitted_classes`

Defaults to `[Symbol]`. Allows applications to include additional permitted classes to `safe_load()` on the `ActiveRecord::Coders::YAMLColumn`.

#### `config.active_record.use_yaml_unsafe_load`

Defaults to `false`. Allows applications to opt into using `unsafe_load` on the `ActiveRecord::Coders::YAMLColumn`.

#### `ActiveRecord::ConnectionAdapters::Mysql2Adapter.emulate_booleans`

Controls whether the Active Record MySQL adapter will consider all `tinyint(1)` columns as booleans. Defaults to `true`.

#### `ActiveRecord::ConnectionAdapters::PostgreSQLAdapter.create_unlogged_tables`

Controls whether database tables created by PostgreSQL should be "unlogged", which can speed
up performance but adds a risk of data loss if the database crashes. It is
highly recommended that you do not enable this in a production environment.
Defaults to `false` in all environments.

To enable this for tests:

```ruby
# config/environments/test.rb

ActiveSupport.on_load(:active_record_postgresqladapter) do
  self.create_unlogged_tables = true
end
```

#### `ActiveRecord::ConnectionAdapters::PostgreSQLAdapter.datetime_type`

Controls what native type the Active Record PostgreSQL adapter should use when you call `datetime` in
a migration or schema. It takes a symbol which must correspond to one of the
configured `NATIVE_DATABASE_TYPES`. The default is `:timestamp`, meaning
`t.datetime` in a migration will create a "timestamp without time zone" column.

To use "timestamp with time zone":

```ruby
# config/application.rb

ActiveSupport.on_load(:active_record_postgresqladapter) do
  self.datetime_type = :timestamptz
end
```

You should run `bin/rails db:migrate` to rebuild your schema.rb if you change this.

#### `ActiveRecord::SchemaDumper.ignore_tables`

Accepts an array of tables that should _not_ be included in any generated schema file.

#### `ActiveRecord::SchemaDumper.fk_ignore_pattern`

Allows setting a different regular expression that will be used to decide
whether a foreign key's name should be dumped to db/schema.rb or not. By
default, foreign key names starting with `fk_rails_` are not exported to the
database schema dump. Defaults to `/^fk_rails_[0-9a-f]{10}$/`.

### Configuring Action Controller

`config.action_controller` includes a number of configuration settings:

#### `config.action_controller.asset_host`

Sets the host for the assets. Useful when CDNs are used for hosting assets rather than the application server itself. You should only use this if you have a different configuration for Action Mailer, otherwise use `config.asset_host`.

#### `config.action_controller.perform_caching`

Configures whether the application should perform the caching features provided by the Action Controller component or not. Set to `false` in the development environment, `true` in production. If it's not specified, the default will be `true`.

#### `config.action_controller.default_static_extension`

Configures the extension used for cached pages. Defaults to `.html`.

#### `config.action_controller.include_all_helpers`

Configures whether all view helpers are available everywhere or are scoped to the corresponding controller. If set to `false`, `UsersHelper` methods are only available for views rendered as part of `UsersController`. If `true`, `UsersHelper` methods are available everywhere. The default configuration behavior (when this option is not explicitly set to `true` or `false`) is that all view helpers are available to each controller.

#### `config.action_controller.logger`

Accepts a logger conforming to the interface of Log4r or the default Ruby Logger class, which is then used to log information from Action Controller. Set to `nil` to disable logging.

#### `config.action_controller.request_forgery_protection_token`

Sets the token parameter name for RequestForgery. Calling `protect_from_forgery` sets it to `:authenticity_token` by default.

#### `config.action_controller.allow_forgery_protection`

Enables or disables CSRF protection. By default this is `false` in the test environment and `true` in all other environments.

#### `config.action_controller.forgery_protection_origin_check`

Configures whether the HTTP `Origin` header should be checked against the site's origin as an additional CSRF defense.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 5.0                   | `true`               |

#### `config.action_controller.per_form_csrf_tokens`

Configures whether CSRF tokens are only valid for the method/action they were generated for.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 5.0                   | `true`               |

#### `config.action_controller.default_protect_from_forgery`

Determines whether forgery protection is added on `ActionController::Base`.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 5.2                   | `true`               |

#### `config.action_controller.relative_url_root`

Can be used to tell Rails that you are [deploying to a subdirectory](
configuring.html#deploy-to-a-subdirectory-relative-url-root). The default is
[`config.relative_url_root`](#config-relative-url-root).

#### `config.action_controller.permit_all_parameters`

Sets all the parameters for mass assignment to be permitted by default. The default value is `false`.

#### `config.action_controller.action_on_unpermitted_parameters`

Controls behavior when parameters that are not explicitly permitted are found. The default value is `:log` in test and development environments, `false` otherwise. The values can be:

* `false` to take no action
* `:log` to emit an `ActiveSupport::Notifications.instrument` event on the `unpermitted_parameters.action_controller` topic and log at the DEBUG level
* `:raise` to raise a `ActionController::UnpermittedParameters` exception

#### `config.action_controller.always_permitted_parameters`

Sets a list of permitted parameters that are permitted by default. The default values are `['controller', 'action']`.

#### `config.action_controller.enable_fragment_cache_logging`

Determines whether to log fragment cache reads and writes in verbose format as follows:

```
Read fragment views/v1/2914079/v1/2914079/recordings/70182313-20160225015037000000/d0bdf2974e1ef6d31685c3b392ad0b74 (0.6ms)
Rendered messages/_message.html.erb in 1.2 ms [cache hit]
Write fragment views/v1/2914079/v1/2914079/recordings/70182313-20160225015037000000/3b4e249ac9d168c617e32e84b99218b5 (1.1ms)
Rendered recordings/threads/_thread.html.erb in 1.5 ms [cache miss]
```

By default it is set to `false` which results in following output:

```
Rendered messages/_message.html.erb in 1.2 ms [cache hit]
Rendered recordings/threads/_thread.html.erb in 1.5 ms [cache miss]
```

#### `config.action_controller.raise_on_open_redirects`

Raises an `ActionController::Redirecting::UnsafeRedirectError` when an unpermitted open redirect occurs.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 7.0                   | `true`               |

#### `config.action_controller.log_query_tags_around_actions`

Determines whether controller context for query tags will be automatically
updated via an `around_filter`. The default value is `true`.

#### `config.action_controller.wrap_parameters_by_default`

Configures the [`ParamsWrapper`](https://api.rubyonrails.org/classes/ActionController/ParamsWrapper.html) to wrap json
request by default.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 7.0                   | `true`               |

#### `ActionController::Base.wrap_parameters`

Configures the [`ParamsWrapper`](https://api.rubyonrails.org/classes/ActionController/ParamsWrapper.html). This can be called at
the top level, or on individual controllers.

#### `config.action_controller.allow_deprecated_parameters_hash_equality`

Controls behavior of `ActionController::Parameters#==` with `Hash` arguments.
Value of the setting determines whether an `ActionController::Parameters` instance is equal to an equivalent `Hash`.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `true`               |
| 7.1                   | `false`              |

### Configuring Action Dispatch

#### `config.action_dispatch.cookies_serializer`

Specifies which serializer to use for cookies. For more information, see [Action Controller Cookies](action_controller_overview.html#cookies).

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `:marshal`           |
| 7.0                   | `:json`              |

#### `config.action_dispatch.default_headers`

Is a hash with HTTP headers that are set by default in each response.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | <pre><code>{<br>  "X-Frame-Options" => "SAMEORIGIN",<br>  "X-XSS-Protection" => "1; mode=block",<br>  "X-Content-Type-Options" => "nosniff",<br>  "X-Download-Options" => "noopen",<br>  "X-Permitted-Cross-Domain-Policies" => "none",<br>  "Referrer-Policy" => "strict-origin-when-cross-origin"<br>}</code></pre> |
| 7.0                   | <pre><code>{<br>  "X-Frame-Options" => "SAMEORIGIN",<br>  "X-XSS-Protection" => "0",<br>  "X-Content-Type-Options" => "nosniff",<br>  "X-Download-Options" => "noopen",<br>  "X-Permitted-Cross-Domain-Policies" => "none",<br>  "Referrer-Policy" => "strict-origin-when-cross-origin"<br>}</code></pre> |
| 7.1                   | <pre><code>{<br>  "X-Frame-Options" => "SAMEORIGIN",<br>  "X-XSS-Protection" => "0",<br>  "X-Content-Type-Options" => "nosniff",<br>  "X-Permitted-Cross-Domain-Policies" => "none",<br>  "Referrer-Policy" => "strict-origin-when-cross-origin"<br>}</code></pre> |

#### `config.action_dispatch.default_charset`

Specifies the default character set for all renders. Defaults to `nil`.

#### `config.action_dispatch.tld_length`

Sets the TLD (top-level domain) length for the application. Defaults to `1`.

#### `config.action_dispatch.ignore_accept_header`

Is used to determine whether to ignore accept headers from a request. Defaults to `false`.

#### `config.action_dispatch.x_sendfile_header`

Specifies server specific X-Sendfile header. This is useful for accelerated file sending from server. For example it can be set to 'X-Sendfile' for Apache.

#### `config.action_dispatch.http_auth_salt`

Sets the HTTP Auth salt value. Defaults
to `'http authentication'`.

#### `config.action_dispatch.signed_cookie_salt`

Sets the signed cookies salt value.
Defaults to `'signed cookie'`.

#### `config.action_dispatch.encrypted_cookie_salt`

Sets the encrypted cookies salt value. Defaults to `'encrypted cookie'`.

#### `config.action_dispatch.encrypted_signed_cookie_salt`

Sets the signed encrypted cookies salt value. Defaults to `'signed encrypted
cookie'`.

#### `config.action_dispatch.authenticated_encrypted_cookie_salt`

Sets the authenticated encrypted cookie salt. Defaults to `'authenticated
encrypted cookie'`.

#### `config.action_dispatch.encrypted_cookie_cipher`

Sets the cipher to be used for encrypted cookies. This defaults to
`"aes-256-gcm"`.

#### `config.action_dispatch.signed_cookie_digest`

Sets the digest to be used for signed cookies. This defaults to `"SHA1"`.

#### `config.action_dispatch.cookies_rotations`

Allows rotating secrets, ciphers, and digests for encrypted and signed cookies.

#### `config.action_dispatch.use_authenticated_cookie_encryption`

Controls whether signed and encrypted cookies use the AES-256-GCM cipher or the
older AES-256-CBC cipher.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 5.2                   | `true`               |

#### `config.action_dispatch.use_cookies_with_metadata`

Enables writing cookies with the purpose metadata embedded.

The default value depends on the `config.load_defaults` target version:

| Starting with version | The default value is |
| --------------------- | -------------------- |
| (original)            | `false`              |
| 6.0                   | `true`               |

#### `config.action_dispatch.perform_deep_munge`

Configures whether `deep_munge` method should be performed on the parameters.
See [Security Guide](security.html#unsafe-query-generation) for more
information. It defaults to `true`.

#### `config.action_dispatch.rescue_responses`

Configures what exceptions are assigned to an HTTP status. It accepts a hash and you can specify pairs of exception/status. By default, this is defined as:

```ruby
config.action_dispatch.rescue_responses = {
  'ActionController::RoutingError'
    => :not_found,
  'AbstractController::ActionNotFound'
    => :not_found,
  'ActionController::MethodNotAllowed'
    => :method_not_allowed,
  'ActionController::UnknownHttpMethod'
    => :method_not_allowed,
  'ActionController::NotImplemented'
    => :not_implemented,
  'ActionController::UnknownFormat'
    => :not_acceptable,
  'ActionController::InvalidAuthenticityToken'
    => :unprocessable_entity,
  'ActionController::InvalidCrossOriginRequest'
    => :unprocessable_entity,
  'ActionDispatch::Http::Parameters::ParseError'
    => :bad_request,
  'ActionController::BadRequest'
    => :bad_request,
  'ActionController::ParameterMissing'
    => :bad_request,
  'Rack::QueryParser::ParameterTypeError'
    => :bad_request,
  'Rack::QueryParser::InvalidParameterError'
    => :bad_request,
  'ActiveRecord::RecordNotFound'
    => :not_found,
  'ActiveRecord::StaleObjectError'
    => :conflict,
  'ActiveRecord::RecordInvalid'
    => :unprocessable_entity,
  'ActiveRecord::RecordNotSaved'
    => :unprocessable_entity
}