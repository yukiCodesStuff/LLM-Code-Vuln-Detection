  {
    host: 'a.com',
    cert: { },
    error: 'Cert does not contain a DNS name'
  },

  // Empty Subject w/DNS name
  {
  {
    host: 'a.b.a.com', cert: {
      subjectaltname: 'URI:http://a.b.a.com/',
    },
    error: 'Cert does not contain a DNS name'
  },

  // Multiple CN fields
  {
    host: 'a.b.a.com', cert: {
      subjectaltname: 'URI:http://a.b.a.com/',
      subject: {}
    },
    error: 'Cert does not contain a DNS name'
  },
  {
    host: 'a.b.a.com', cert: {
      subjectaltname: 'URI:http://*.b.a.com/',
      subject: {}
    },
    error: 'Cert does not contain a DNS name'
  },
  // IP addresses
  {
    host: 'a.b.a.com', cert: {
      subjectaltname: 'IP Address:127.0.0.1',
      subject: {}
    },
    error: 'Cert does not contain a DNS name'
  },
  {
    host: '127.0.0.1', cert: {
      subjectaltname: 'IP Address:127.0.0.1',