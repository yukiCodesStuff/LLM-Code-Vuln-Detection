* `config.action_dispatch.encrypted_signed_cookie_salt` sets the signed
encrypted cookies salt value. Defaults to `'signed encrypted cookie'`.

* `config.action_dispatch.perform_deep_munge` configures whether `deep_munge`
  method should be performed on the parameters. See [Security Guide](security.html#unsafe-query-generation)
  for more information. It defaults to true.

* `ActionDispatch::Callbacks.before` takes a block of code to run before the request.

* `ActionDispatch::Callbacks.to_prepare` takes a block to run after `ActionDispatch::Callbacks.before`, but before the request. Runs for every request in `development` mode, but only once for `production` or environments with `cache_classes` set to `true`.
