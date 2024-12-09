    $('#qunit-fixture').append($('<button />', {
      text: 'Click me',
      'data-remote': true,
      'data-url': '/echo',
      'data-disable-with': 'clicking...'
    }))
  },
  afterEach: function() {
    $(document).unbind('iframe:loaded')
  }
      setTimeout(function() {
        assert.enabledState(button, 'Click me')
        done()
      }, 30)
    })
    .triggerNative('click')
})