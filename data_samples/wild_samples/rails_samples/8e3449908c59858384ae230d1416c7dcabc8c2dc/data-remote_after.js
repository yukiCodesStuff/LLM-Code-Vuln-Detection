      .append($('<a />', {
        href: '/echo',
        'data-remote': 'true',
        disabled: 'disabled',
        text: 'Disabled link'
      }))
      .find('form').append($('<input type="text" name="user_name" value="john">'))

    $('#qunit-fixture').append($('<div />', {
      id: 'edit-div', 'contenteditable': 'true'
    }))
  }
    .bindNative('ajax:success', function(e, data, status, xhr) {
      assert.equal(data.params.user_name, 'john')
      assert.equal(data.params.description, 'A wise man')
      assert.equal(data.params.age, undefined)

      done()
    })
    .triggerNative('submit')
})



QUnit.test('clicking on a link with contenteditable attribute does not fire ajaxyness', function(assert) {
  const done = assert.async()
  assert.expect(0)

  var contenteditable_div = $('#qunit-fixture').find('div')
  var link = $('a[data-remote]')
  contenteditable_div.append(link)

  link
    .bindNative('ajax:beforeSend', function() {
      assert.ok(false, 'ajax should not be triggered')
    })
    .bindNative('click', function(e) {
      e.preventDefault()
    })
    .triggerNative('click')

  setTimeout(function() { done() }, 20)
})