      'data-url': '/echo',
      'data-disable-with': 'clicking...'
    }))

    $('#qunit-fixture').append($('<div />', {
      id: 'edit-div', 'contenteditable': 'true'
    }))
  },
  afterEach: function() {
    $(document).unbind('iframe:loaded')
  }
    })
    .triggerNative('click')
})

QUnit.test('form button with "data-disable-with" attribute and contenteditable is not modified', function(assert) {
  const done = assert.async()
  var form = $('form[data-remote]'), button = $('<button data-disable-with="submitting ..." name="submit2">Submit</button>')

  var contenteditable_div = $('#qunit-fixture').find('div')
  form.append(button)
  contenteditable_div.append(form)

  assert.enabledState(button, 'Submit')

  setTimeout(function() {
    assert.enabledState(button, 'Submit')
    done()
  }, 13)
  form.triggerNative('submit')

  assert.enabledState(button, 'Submit')
})