// Flags: --experimental-network-imports --dns-result-order=ipv4first
import * as common from '../common/index.mjs';
import * as fixtures from '../common/fixtures.mjs';
import tmpdir from '../common/tmpdir.js';
import assert from 'assert';
import http from 'http';
import os from 'os';
import util from 'util';
import { describe, it } from 'node:test';

if (!common.hasCrypto) {
  common.skip('missing crypto');
}
tmpdir.refresh();

const https = (await import('https')).default;

const createHTTPServer = http.createServer;
// Needed to deal w/ test certs
process.env.NODE_TLS_REJECT_UNAUTHORIZED = '0';
const options = {
  key: fixtures.readKey('agent1-key.pem'),
  cert: fixtures.readKey('agent1-cert.pem')
};

const createHTTPSServer = https.createServer.bind(null, options);

      url.href + 'bar/baz.js'
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

    await describe('guarantee data url will not bypass import restriction', () => {
      it('should not be bypassed by cross protocol redirect', async () => {
        const crossProtocolRedirect = new URL(url.href);
        crossProtocolRedirect.searchParams.set('redirect', JSON.stringify({
          status: 302,
          location: 'data:text/javascript,'
        }));
        await assert.rejects(
          import(crossProtocolRedirect.href),
          { code: 'ERR_NETWORK_IMPORT_DISALLOWED' }
        );
      });

      it('should not be bypassed by data URL', async () => {
        const deps = new URL(url.href);
        deps.searchParams.set('body', `
        export {data} from 'data:text/javascript,export let data = 1';
        import * as http from ${JSON.stringify(url.href)};
        export {http};
      `);
        await assert.rejects(
          import(deps.href),
          { code: 'ERR_NETWORK_IMPORT_DISALLOWED' }
        );
      });

      it('should not be bypassed by encodedURI import', async () => {
        const deepDataImport = new URL(url.href);
        deepDataImport.searchParams.set('body', `
        import 'data:text/javascript,import${encodeURIComponent(JSON.stringify('data:text/javascript,import "os"'))}';
      `);
        await assert.rejects(
          import(deepDataImport.href),
          { code: 'ERR_NETWORK_IMPORT_DISALLOWED' }
        );
      });

      it('should not be bypassed by relative deps import', async () => {
        const relativeDeps = new URL(url.href);
        relativeDeps.searchParams.set('body', `
        import * as http from "./";
        export {http};
      `);
        const relativeDepsNS = await import(relativeDeps.href);
        assert.strict.deepStrictEqual(Object.keys(relativeDepsNS), ['http']);
        assert.strict.equal(relativeDepsNS.http, ns);
      });

      it('should not be bypassed by file dependency import', async () => {
        const fileDep = new URL(url.href);
        const { href } = fixtures.fileURL('/es-modules/message.mjs');
        fileDep.searchParams.set('body', `
        import ${JSON.stringify(href)};
        export default 1;`);
        await assert.rejects(
          import(fileDep.href),
          { code: 'ERR_NETWORK_IMPORT_DISALLOWED' }
        );
      });

      it('should not be bypassed by builtin dependency import', async () => {
        const builtinDep = new URL(url.href);
        builtinDep.searchParams.set('body', `
        import 'node:fs';
        export default 1;
      `);
        await assert.rejects(
          import(builtinDep.href),
          { code: 'ERR_NETWORK_IMPORT_DISALLOWED' }
        );
      });


      it('should not be bypassed by unprefixed builtin dependency import', async () => {
        const unprefixedBuiltinDep = new URL(url.href);
        unprefixedBuiltinDep.searchParams.set('body', `
        import 'fs';
        export default 1;
      `);
        await assert.rejects(
          import(unprefixedBuiltinDep.href),
          { code: 'ERR_NETWORK_IMPORT_DISALLOWED' }
        );
      });

      it('should not be bypassed by indirect network import', async () => {
        const indirect = new URL(url.href);
        indirect.searchParams.set('body', `
        import childProcess from 'data:text/javascript,export { default } from "node:child_process"'
        export {childProcess};
      `);
        await assert.rejects(
          import(indirect.href),
          { code: 'ERR_NETWORK_IMPORT_DISALLOWED' }
        );
      });

      it('data: URL can always import other data:', async () => {
        const data = new URL('data:text/javascript,');
        data.searchParams.set('body',
                              'import \'data:text/javascript,import \'data:\''
        );
        // doesn't throw
        const empty = await import(data.href);
        assert.ok(empty);
      });

      it('data: URL cannot import file: or builtin', async () => {
        const data1 = new URL(url.href);
        data1.searchParams.set('body',
                               'import \'file:///some/file.js\''
        );
        await assert.rejects(
          import(data1.href),
          { code: 'ERR_NETWORK_IMPORT_DISALLOWED' }
        );

        const data2 = new URL(url.href);
        data2.searchParams.set('body',
                               'import \'node:fs\''
        );
        await assert.rejects(
          import(data2.href),
          { code: 'ERR_NETWORK_IMPORT_DISALLOWED' }
        );
      });

      it('data: URL cannot import HTTP URLs', async () => {
        const module = fixtures.fileURL('/es-modules/import-data-url.mjs');
        try {
          await import(module);
        } catch (err) {
          // We only want the module to load, we don't care if the module throws an
          // error as long as the loader does not.
          assert.notStrictEqual(err?.code, 'ERR_MODULE_NOT_FOUND');
        }
        const data1 = new URL(url.href);
        const dataURL = 'data:text/javascript;export * from "node:os"';
        data1.searchParams.set('body', `export * from ${JSON.stringify(dataURL)};`);
        await assert.rejects(
          import(data1),
          { code: 'ERR_NETWORK_IMPORT_DISALLOWED' }
        );
      });
    });

    server.close();
  }
}