      'data-url': '/echo',
      'data-disable-with': 'clicking...'
    }))
  },
  afterEach: function() {
    $(document).unbind('iframe:loaded')
  }
    })
    .triggerNative('click')
})