
`config.active_storage` provides the following configuration options:

#### `config.active_record.yaml_column_permitted_classes`

Defaults to `[]`. Allows applications to include additional permitted classes to `safe_load()` on the `ActiveStorage::Coders::YamlColumn`.

#### `config.active_storage.use_yaml_unsafe_load`

Defaults to `false`. Allows applications to opt into using `unsafe_load` on the `ActiveStorage::Coders::YamlColumn`.

#### `config.active_storage.variant_processor`

Accepts a symbol `:mini_magick` or `:vips`, specifying whether variant transformations and blob analysis will be performed with MiniMagick or ruby-vips.
