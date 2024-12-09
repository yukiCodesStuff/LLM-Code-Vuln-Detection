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