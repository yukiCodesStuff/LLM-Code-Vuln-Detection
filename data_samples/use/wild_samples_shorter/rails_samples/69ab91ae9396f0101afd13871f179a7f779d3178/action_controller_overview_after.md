
The value of `params[:ids]` will now be `["1", "2", "3"]`. Note that parameter values are always strings; Rails makes no attempt to guess or cast the type.

NOTE: Values such as `[]`, `[nil]` or `[nil, nil, ...]` in `params` are replaced
with `nil` for security reasons by default. See [Security Guide](security.html#unsafe-query-generation)
for more information.

To send a hash you include the key name inside the brackets:

```html
<form accept-charset="UTF-8" action="/clients" method="post">