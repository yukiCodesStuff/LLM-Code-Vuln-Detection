  http.get({
    host: '127.0.0.1',
    port: this.address().port
  }, common.mustCall(function(res) {
    assert.strictEqual(res.headers['content-type'],
                       'text/plain; x-unix-mode=0600; name="hello.txt"');
    res.destroy();
  }));
}));