// Flags: --experimental-network-imports --dns-result-order=ipv4first
import * as common from '../common/index.mjs';
import { path, readKey } from '../common/fixtures.mjs';
import { pathToFileURL } from 'url';
import assert from 'assert';
import http from 'http';
import os from 'os';
import util from 'util';

if (!common.hasCrypto) {
  common.skip('missing crypto');
}

const https = (await import('https')).default;

const createHTTPServer = http.createServer;
// Needed to deal w/ test certs
process.env.NODE_TLS_REJECT_UNAUTHORIZED = '0';
const options = {
  key: readKey('agent1-key.pem'),
  cert: readKey('agent1-cert.pem')
};

const createHTTPSServer = https.createServer.bind(null, options);

      url.href + 'bar/baz.js'
    );

    const crossProtocolRedirect = new URL(url.href);
    crossProtocolRedirect.searchParams.set('redirect', JSON.stringify({
      status: 302,
      location: 'data:text/javascript,'
    }));
    await assert.rejects(
      import(crossProtocolRedirect.href),
      { code: 'ERR_NETWORK_IMPORT_DISALLOWED' }
    );

    const deps = new URL(url.href);
    deps.searchParams.set('body', `
      export {data} from 'data:text/javascript,export let data = 1';
      import * as http from ${JSON.stringify(url.href)};
      export {http};
    `);
    const depsNS = await import(deps.href);
    assert.strict.deepStrictEqual(Object.keys(depsNS), ['data', 'http']);
    assert.strict.equal(depsNS.data, 1);
    assert.strict.equal(depsNS.http, ns);

    const relativeDeps = new URL(url.href);
    relativeDeps.searchParams.set('body', `
      import * as http from "./";
      export {http};
    `);
    const relativeDepsNS = await import(relativeDeps.href);
    assert.strict.deepStrictEqual(Object.keys(relativeDepsNS), ['http']);
    assert.strict.equal(relativeDepsNS.http, ns);
    const fileDep = new URL(url.href);
    const { href } = pathToFileURL(path('/es-modules/message.mjs'));
    fileDep.searchParams.set('body', `
      import ${JSON.stringify(href)};
      export default 1;`);
    await assert.rejects(
      import(fileDep.href),
      { code: 'ERR_NETWORK_IMPORT_DISALLOWED' }
    );

    const builtinDep = new URL(url.href);
    builtinDep.searchParams.set('body', `
      import 'node:fs';
      export default 1;
    `);
    await assert.rejects(
      import(builtinDep.href),
      { code: 'ERR_NETWORK_IMPORT_DISALLOWED' }
    );

    const unprefixedBuiltinDep = new URL(url.href);
    unprefixedBuiltinDep.searchParams.set('body', `
      import 'fs';
      export default 1;
    `);
    await assert.rejects(
      import(unprefixedBuiltinDep.href),
      { code: 'ERR_NETWORK_IMPORT_DISALLOWED' }
    );

    const unsupportedMIME = new URL(url.href);
    unsupportedMIME.searchParams.set('mime', 'application/node');
    unsupportedMIME.searchParams.set('body', '');
    await assert.rejects(
      import(unsupportedMIME.href),
      { code: 'ERR_UNKNOWN_MODULE_FORMAT' }
    );
    const notFound = new URL(url.href);
    notFound.pathname = '/not-found';
    await assert.rejects(
      import(notFound.href),
    assert.deepStrictEqual(Object.keys(json), ['default']);
    assert.strictEqual(json.default.x, 1);

    server.close();
  }
}