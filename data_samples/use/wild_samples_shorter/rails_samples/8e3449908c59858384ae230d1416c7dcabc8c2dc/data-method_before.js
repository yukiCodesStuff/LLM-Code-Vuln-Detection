    $('#qunit-fixture').append($('<a />', {
      href: '/echo', 'data-method': 'delete', text: 'destroy!'
    }))
  },
  afterEach: function() {
    $(document).unbind('iframe:loaded')
  }

  assert.notEqual(data.authenticity_token, 'cf50faa3fe97702ca1ae')
})