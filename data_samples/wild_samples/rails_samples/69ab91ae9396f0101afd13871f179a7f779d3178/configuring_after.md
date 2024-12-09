    config.action_dispatch.default_headers = {
      'X-Frame-Options' => 'SAMEORIGIN',
      'X-XSS-Protection' => '1; mode=block',
      'X-Content-Type-Options' => 'nosniff'
    }
    ```

* `config.action_dispatch.tld_length` sets the TLD (top-level domain) length for the application. Defaults to `1`.

* `config.action_dispatch.http_auth_salt` sets the HTTP Auth salt value. Defaults
to `'http authentication'`.

* `config.action_dispatch.signed_cookie_salt` sets the signed cookies salt value.
Defaults to `'signed cookie'`.

* `config.action_dispatch.encrypted_cookie_salt` sets the encrypted cookies salt
value. Defaults to `'encrypted cookie'`.

* `config.action_dispatch.encrypted_signed_cookie_salt` sets the signed
encrypted cookies salt value. Defaults to `'signed encrypted cookie'`.

* `config.action_dispatch.perform_deep_munge` configures whether `deep_munge`
  method should be performed on the parameters. See [Security Guide](security.html#unsafe-query-generation)
  for more information. It defaults to true.

* `ActionDispatch::Callbacks.before` takes a block of code to run before the request.

* `ActionDispatch::Callbacks.to_prepare` takes a block to run after `ActionDispatch::Callbacks.before`, but before the request. Runs for every request in `development` mode, but only once for `production` or environments with `cache_classes` set to `true`.

* `ActionDispatch::Callbacks.after` takes a block of code to run after the request.

### Configuring Action View

`config.action_view` includes a small number of configuration settings:

* `config.action_view.field_error_proc` provides an HTML generator for displaying errors that come from Active Record. The default is

    ```ruby
    Proc.new do |html_tag, instance|
      %Q(<div class="field_with_errors">#{html_tag}</div>).html_safe
    end
    ```

* `config.action_view.default_form_builder` tells Rails which form builder to use by default. The default is `ActionView::Helpers::FormBuilder`. If you want your form builder class to be loaded after initialization (so it's reloaded on each request in development), you can pass it as a `String`

* `config.action_view.logger` accepts a logger conforming to the interface of Log4r or the default Ruby Logger class, which is then used to log information from Action View. Set to `nil` to disable logging.

* `config.action_view.erb_trim_mode` gives the trim mode to be used by ERB. It defaults to `'-'`. See the [ERB documentation](http://www.ruby-doc.org/stdlib/libdoc/erb/rdoc/) for more information.

* `config.action_view.embed_authenticity_token_in_remote_forms` allows you to set the default behavior for `authenticity_token` in forms with `:remote => true`. By default it's set to false, which means that remote forms will not include `authenticity_token`, which is helpful when you're fragment-caching the form. Remote forms get the authenticity from the `meta` tag, so embedding is unnecessary unless you support browsers without JavaScript. In such case you can either pass `:authenticity_token => true` as a form option or set this config setting to `true`

* `config.action_view.prefix_partial_path_with_controller_namespace` determines whether or not partials are looked up from a subdirectory in templates rendered from namespaced controllers. For example, consider a controller named `Admin::PostsController` which renders this template:

    ```erb
    <%= render @post %>
    ```

    The default setting is `true`, which uses the partial at `/admin/posts/_post.erb`. Setting the value to `false` would render `/posts/_post.erb`, which is the same behavior as rendering from a non-namespaced controller such as `PostsController`.

* `config.action_view.raise_on_missing_translations` determines whether an error should be raised for missing translations

### Configuring Action Mailer

There are a number of settings available on `config.action_mailer`:

* `config.action_mailer.logger` accepts a logger conforming to the interface of Log4r or the default Ruby Logger class, which is then used to log information from Action Mailer. Set to `nil` to disable logging.

* `config.action_mailer.smtp_settings` allows detailed configuration for the `:smtp` delivery method. It accepts a hash of options, which can include any of these options:
    * `:address` - Allows you to use a remote mail server. Just change it from its default "localhost" setting.
    * `:port` - On the off chance that your mail server doesn't run on port 25, you can change it.
    * `:domain` - If you need to specify a HELO domain, you can do it here.
    * `:user_name` - If your mail server requires authentication, set the username in this setting.
    * `:password` - If your mail server requires authentication, set the password in this setting.
    * `:authentication` - If your mail server requires authentication, you need to specify the authentication type here. This is a symbol and one of `:plain`, `:login`, `:cram_md5`.

* `config.action_mailer.sendmail_settings` allows detailed configuration for the `sendmail` delivery method. It accepts a hash of options, which can include any of these options:
    * `:location` - The location of the sendmail executable. Defaults to `/usr/sbin/sendmail`.
    * `:arguments` - The command line arguments. Defaults to `-i -t`.

* `config.action_mailer.raise_delivery_errors` specifies whether to raise an error if email delivery cannot be completed. It defaults to true.

* `config.action_mailer.delivery_method` defines the delivery method and defaults to `:smtp`. See the [configuration section in the Action Mailer guide](http://guides.rubyonrails.org/action_mailer_basics.html#action-mailer-configuration) for more info.

* `config.action_mailer.perform_deliveries` specifies whether mail will actually be delivered and is true by default. It can be convenient to set it to false for testing.

* `config.action_mailer.default_options` configures Action Mailer defaults. Use to set options like `from` or `reply_to` for every mailer. These default to:

    ```ruby
    mime_version:  "1.0",
    charset:       "UTF-8",
    content_type: "text/plain",
    parts_order:  ["text/plain", "text/enriched", "text/html"]
    ```

    Assign a hash to set additional options:

    ```ruby
    config.action_mailer.default_options = {
      from: "noreply@example.com"
    }