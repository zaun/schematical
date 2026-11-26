// A stand-in for globalThis.fetch, loaded with `node --require` before
// scripts/check-datasheets.js (check-datasheets.test.js), so no test touches
// the network. CHECK_STUB is JSON: { "<url>": "<kind>" }; an unlisted URL
// serves a PDF. Kinds:
//   pdf        a 2 KB PDF in 512-byte chunks (the reader stops after 1 KB)
//   moved      a PDF whose response.url is another URL (a redirect)
//   short      "%PDF-1.4" and the end of the body (fewer than 1 KB)
//   offset     a few bytes, then the PDF header in a later chunk
//   http404    an HTTP 404
//   html       an HTML page with a content-type
//   bare       a body that is not a PDF and no content-type
//   cancel     a PDF whose reader.cancel() rejects
//   error      fetch rejects with "connect ECONNREFUSED"
//   hang       never answers until the request's signal aborts

'use strict';

const kinds = JSON.parse(process.env.CHECK_STUB || '{}');

function response(url, { status = 200, type = null, chunks = [], cancel = null, finalUrl = url }) {
  let index = 0;
  return {
    ok: status >= 200 && status < 300,
    status,
    url: finalUrl,
    headers: { get: (name) => (name.toLowerCase() === 'content-type' ? type : null) },
    body: {
      getReader: () => ({
        read: async () => (index < chunks.length ? { done: false, value: new TextEncoder().encode(chunks[index++]) } : { done: true }),
        cancel: async () => { if (cancel) throw new Error(cancel); },
      }),
    },
  };
}

const PDF = ['%PDF-1.7\n' + 'x'.repeat(503), 'y'.repeat(512), 'z'.repeat(512), 'w'.repeat(512)];

globalThis.fetch = (url, init) => {
  const kind = kinds[url] || 'pdf';
  if (init.redirect !== 'follow' || !/Schematical datasheet check/.test(init.headers['User-Agent'])) {
    return Promise.reject(new Error('the stub got unexpected options'));
  }
  switch (kind) {
    case 'pdf': return Promise.resolve(response(url, { type: 'application/pdf', chunks: PDF }));
    case 'moved': return Promise.resolve(response(url, { type: 'application/pdf', chunks: PDF, finalUrl: url + '?moved' }));
    case 'short': return Promise.resolve(response(url, { type: 'application/pdf', chunks: ['%PDF-1.4'] }));
    case 'offset': return Promise.resolve(response(url, { chunks: ['\r\n\r\n', '%PDF-1.3 ...'] }));
    case 'http404': return Promise.resolve(response(url, { status: 404 }));
    case 'html': return Promise.resolve(response(url, { type: 'text/html', chunks: ['<html>no</html>'] }));
    case 'bare': return Promise.resolve(response(url, { chunks: ['GIF89a'] }));
    case 'cancel': return Promise.resolve(response(url, { type: 'application/pdf', chunks: PDF, cancel: 'cannot cancel' }));
    case 'error': return Promise.reject(new Error('connect ECONNREFUSED'));
    case 'hang': return new Promise((resolve, reject) => {
      init.signal.addEventListener('abort', () => {
        const error = new Error('This operation was aborted');
        error.name = 'AbortError';
        reject(error);
      });
    });
    default: return Promise.reject(new Error('unknown stub kind ' + kind));
  }
};
