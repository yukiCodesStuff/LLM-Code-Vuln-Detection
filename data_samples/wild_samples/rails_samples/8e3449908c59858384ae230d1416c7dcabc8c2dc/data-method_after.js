    $('#qunit-fixture').append($('<a />', {
      href: '/echo', 'data-method': 'delete', text: 'destroy!'
    }))

    $('#qunit-fixture').append($('<div />', {
      id: 'edit-div', 'contenteditable': 'true'
    }))
  },
  afterEach: function() {
    $(document).unbind('iframe:loaded')
  }
})

function submit(fn, options) {
  $(document).bind('iframe:loaded', function(e, data) {
    fn(data)
  })

  $('#qunit-fixture').find('a')
    .triggerNative('click')
}

QUnit.test('link with "data-method" set to "delete"', function(assert) {
  const done = assert.async()

  submit(function(data) {
    assert.equal(data.REQUEST_METHOD, 'DELETE')
    assert.strictEqual(data.params.authenticity_token, undefined)
    assert.strictEqual(data.HTTP_X_CSRF_TOKEN, undefined)
    done()
  })
})

QUnit.test('click on the child of link with "data-method"', function(assert) {
  const done = assert.async()

  $(document).bind('iframe:loaded', function(e, data) {
    assert.equal(data.REQUEST_METHOD, 'DELETE')
    assert.strictEqual(data.params.authenticity_token, undefined)
    assert.strictEqual(data.HTTP_X_CSRF_TOKEN, undefined)
    done()
  })
  $('#qunit-fixture a').html('<strong>destroy!</strong>').find('strong').triggerNative('click')
})

QUnit.test('link with "data-method" and CSRF', function(assert) {
  const done = assert.async()

  $('#qunit-fixture')
    .append('<meta name="csrf-param" content="authenticity_token"/>')
    .append('<meta name="csrf-token" content="cf50faa3fe97702ca1ae"/>')

  submit(function(data) {
    assert.equal(data.params.authenticity_token, 'cf50faa3fe97702ca1ae')
    done()
  })
})

QUnit.test('link "target" should be carried over to generated form', function(assert) {
  const done = assert.async()

  $('a[data-method]').attr('target', 'super-special-frame')
  submit(function(data) {
    assert.equal(data.params._target, 'super-special-frame')
    done()
  })
})

QUnit.test('link with "data-method" and cross origin', function(assert) {
  var data = {}

  $('#qunit-fixture')
    .append('<meta name="csrf-param" content="authenticity_token"/>')
    .append('<meta name="csrf-token" content="cf50faa3fe97702ca1ae"/>')

  $(document).on('submit', 'form', function(e) {
    $(e.currentTarget).serializeArray().map(function(item) {
      data[item.name] = item.value
    })

    return false
  })

  var link = $('#qunit-fixture').find('a')

  link.attr('href', 'http://www.alfajango.com')

  link.triggerNative('click')

  assert.notEqual(data.authenticity_token, 'cf50faa3fe97702ca1ae')
})

QUnit.test('do not interact with contenteditable elements', function(assert) {
  var contenteditable_div = $('#qunit-fixture').find('div')
  contenteditable_div.append('<a href="http://www.shouldnevershowindocument.com" data-method="delete">')

  var link = $('#edit-div').find('a')
  link.triggerNative('click')

  var collection = document.getElementsByTagName('form')
  for (const item of collection) {
    assert.notEqual(item.action, "http://www.shouldnevershowindocument.com/")
  }
})
    $(e.currentTarget).serializeArray().map(function(item) {
      data[item.name] = item.value
    })

    return false
  })

  var link = $('#qunit-fixture').find('a')

  link.attr('href', 'http://www.alfajango.com')

  link.triggerNative('click')

  assert.notEqual(data.authenticity_token, 'cf50faa3fe97702ca1ae')
})

QUnit.test('do not interact with contenteditable elements', function(assert) {
  var contenteditable_div = $('#qunit-fixture').find('div')
  contenteditable_div.append('<a href="http://www.shouldnevershowindocument.com" data-method="delete">')

  var link = $('#edit-div').find('a')
  link.triggerNative('click')

  var collection = document.getElementsByTagName('form')
  for (const item of collection) {
    assert.notEqual(item.action, "http://www.shouldnevershowindocument.com/")
  }
})