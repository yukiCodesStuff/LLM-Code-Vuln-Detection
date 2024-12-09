render template: "products/show"
```

#### Rendering an Arbitrary File

The `render` method can also use a view that's entirely outside of your application:

```ruby
render file: "/u/apps/warehouse_app/current/app/views/products/show"
```

The `:file` option takes an absolute file-system path. Of course, you need to have rights
to the view that you're using to render the content.

NOTE: Using the `:file` option in combination with users input can lead to security problems
since an attacker could use this action to access security sensitive files in your file system.

NOTE: By default, the file is rendered using the current layout.

TIP: If you're running Rails on Microsoft Windows, you should use the `:file` option to
render a file, because Windows filenames do not have the same format as Unix filenames.

#### Wrapping it up

The above three ways of rendering (rendering another template within the controller, rendering a template within another controller, and rendering an arbitrary file on the file system) are actually variants of the same action.

NOTE: Unless overridden, your response returned from this render option will be
`text/plain`, as that is the default content type of Action Dispatch response.

#### Options for `render`

Calls to the `render` method generally accept five options:
