render template: "products/show"
```

#### Wrapping it up

The above three ways of rendering (rendering another template within the controller, rendering a template within another controller, and rendering an arbitrary file on the file system) are actually variants of the same action.

NOTE: Unless overridden, your response returned from this render option will be
`text/plain`, as that is the default content type of Action Dispatch response.

#### Rendering raw file

Rails can render a raw file from an absolute path. This is useful for
conditionally rendering static files like error pages.

```ruby
render file: "#{Rails.root}/public/404.html", layout: false
```

This renders the raw file (it doesn't support ERB or other handlers). By
default it is rendered within the current layout.

WARNING: Using the `:file` option in combination with users input can lead to security problems
since an attacker could use this action to access security sensitive files in your file system.

TIP: `send_file` is often a faster and better option if a layout isn't required.

#### Options for `render`

Calls to the `render` method generally accept five options:
