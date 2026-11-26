// A small fake DOM for the node tests of the datasheet window's page scripts (datasheet.js,
// datasheet-edit.js): just the DOM surface those scripts use (plan-68-T § 3), no npm dependency.
//
//   const dom = createDom();          // {window, document, timers, posted, load(file), flush()}
//   dom.load('…/datasheet.js');        // runs a page script in the window, under its own file name
//
// Elements: tagName, children, childNodes, parentNode, attributes, id, className, title, size, type,
// disabled, dataset, style, classList, textContent, innerHTML (set parses HTML; get serializes),
// value, selectionStart/End, setSelectionRange, setRangeText, focus (document.activeElement),
// addEventListener/dispatchEvent (bubbling), on<event> properties, closest, querySelector(All) (tag,
// #id, .class, [attr], [attr="v"], :last-of-type, descendant combinator, comma lists), cloneNode,
// append, before, remove, isConnected, scrollIntoView (recorded), scrollHeight, offsetWidth.
// The window records webkit.messageHandlers.edit/cite messages in `posted`, and its timers run only
// when the test calls flush().

'use strict';

const fs = require('fs');
const vm = require('vm');

const VOID = new Set(['area', 'base', 'br', 'col', 'embed', 'hr', 'img', 'input', 'link', 'meta', 'source', 'wbr']);
const ENTITIES = { amp: '&', lt: '<', gt: '>', quot: '"', apos: "'", nbsp: ' ' };
const PROPERTY_ATTRS = { id: 'id', className: 'class', title: 'title', type: 'type' };

function decode(text) {
  return text.replace(/&(#x[0-9a-f]+|#\d+|\w+);/gi, (whole, name) => {
    if (name[0] === '#') return String.fromCodePoint(name[1] === 'x' || name[1] === 'X' ? parseInt(name.slice(2), 16) : Number(name.slice(1)));
    return ENTITIES[name] !== undefined ? ENTITIES[name] : whole;
  });
}

function escapeText(s) {
  return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
}

class Event {
  constructor(type, init) {
    this.type = type;
    this.bubbles = !!(init && init.bubbles);
    this.defaultPrevented = false;
    this.target = null;
    Object.assign(this, init || {});
  }
  preventDefault() { this.defaultPrevented = true; }
}

class Text {
  constructor(doc, data) { this.ownerDocument = doc; this.nodeType = 3; this.data = String(data); this.parentNode = null; }
  get textContent() { return this.data; }
  cloneNode() { return new Text(this.ownerDocument, this.data); }
  remove() { if (this.parentNode) this.parentNode._detach(this); }
}

class ClassList {
  constructor(el) { this.el = el; }
  _get() { return (this.el.getAttribute('class') || '').split(/\s+/).filter(Boolean); }
  _set(list) { this.el.setAttribute('class', list.join(' ')); }
  contains(c) { return this._get().includes(c); }
  add(...cs) { const l = this._get(); for (const c of cs) if (!l.includes(c)) l.push(c); this._set(l); }
  remove(...cs) { this._set(this._get().filter((c) => !cs.includes(c))); }
  toggle(c, force) {
    const on = force === undefined ? !this.contains(c) : !!force;
    if (on) this.add(c); else this.remove(c);
    return on;
  }
}

function datasetKey(name) { return 'data-' + name.replace(/[A-Z]/g, (m) => '-' + m.toLowerCase()); }

class Element {
  constructor(doc, tag) {
    this.ownerDocument = doc;
    this.nodeType = 1;
    this.tagName = tag.toUpperCase();
    this.localName = tag.toLowerCase();
    this.childNodes = [];
    this.parentNode = null;
    this.attributes = new Map();
    this.style = {};
    this.classList = new ClassList(this);
    this.listeners = {};
    this.value = '';
    this.disabled = false;
    this.selectionStart = 0;
    this.selectionEnd = 0;
    this.scrolledIntoView = 0;
    const el = this;
    this.dataset = new Proxy({}, {
      get: (_, k) => (typeof k === 'string' ? el.getAttribute(datasetKey(k)) ?? undefined : undefined),
      set: (_, k, v) => { el.setAttribute(datasetKey(k), String(v)); return true; },
    });
  }
  get children() { return this.childNodes.filter((n) => n.nodeType === 1); }
  get size() { return Number(this.getAttribute('size')) || 0; }
  set size(v) { this.setAttribute('size', String(v)); }
  get scrollHeight() { return 40; }
  get offsetWidth() { return 100; }
  get isConnected() {
    let n = this;
    while (n.parentNode) n = n.parentNode;
    return n === this.ownerDocument.documentElement;
  }
  getAttribute(name) { return this.attributes.has(name) ? this.attributes.get(name) : null; }
  setAttribute(name, value) { this.attributes.set(name, String(value)); }
  hasAttribute(name) { return this.attributes.has(name); }
  removeAttribute(name) { this.attributes.delete(name); }

  _detach(node) {
    const i = this.childNodes.indexOf(node);
    if (i >= 0) this.childNodes.splice(i, 1);
    node.parentNode = null;
  }
  _adopt(node) {
    if (typeof node === 'string') return new Text(this.ownerDocument, node);
    if (node.parentNode) node.parentNode._detach(node);
    return node;
  }
  append(...nodes) {
    for (const n of nodes) { const node = this._adopt(n); node.parentNode = this; this.childNodes.push(node); }
  }
  appendChild(node) { this.append(node); return node; }
  before(...nodes) {
    const parent = this.parentNode;
    for (const n of nodes) {
      const node = this._adopt(n);
      node.parentNode = parent;
      parent.childNodes.splice(parent.childNodes.indexOf(this), 0, node);
    }
  }
  remove() { if (this.parentNode) this.parentNode._detach(this); }

  get textContent() { return this.childNodes.map((n) => n.textContent).join(''); }
  set textContent(v) {
    for (const n of this.childNodes) n.parentNode = null;
    this.childNodes = [];
    if (v !== '' && v != null) this.append(String(v));
  }
  get innerHTML() { return this.childNodes.map(serialize).join(''); }
  set innerHTML(html) {
    this.textContent = '';
    parseInto(this, String(html));
  }
  get outerHTML() { return serialize(this); }

  cloneNode(deep) {
    const copy = new Element(this.ownerDocument, this.localName);
    for (const [k, v] of this.attributes) copy.attributes.set(k, v);
    copy.value = this.value;
    if (deep) for (const n of this.childNodes) copy.append(n.cloneNode(true));
    return copy;
  }

  addEventListener(type, fn) { (this.listeners[type] = this.listeners[type] || []).push(fn); }
  dispatchEvent(event) {
    if (!event.target) event.target = this;
    for (let n = this; n; n = event.bubbles ? n.parentNode : null) {
      event.currentTarget = n;
      for (const fn of n.listeners[event.type] || []) fn.call(n, event);
      if (typeof n['on' + event.type] === 'function') n['on' + event.type](event);
    }
    return !event.defaultPrevented;
  }
  /** Test helper: a click (bubbling) on this element. */
  click() { return this.dispatchEvent(new Event('click', { bubbles: true })); }

  focus() {
    this.ownerDocument.activeElement = this;
    this.dispatchEvent(new Event('focus'));
  }
  scrollIntoView() { this.scrolledIntoView++; }
  setSelectionRange(start, end) { this.selectionStart = start; this.selectionEnd = end; }
  setRangeText(text, start, end) {
    this.value = this.value.slice(0, start) + text + this.value.slice(end);
    this.selectionStart = start;
    this.selectionEnd = start + text.length;
  }

  matches(selector) { return parseSelector(selector).some((chain) => matchChain(this, chain)); }
  closest(selector) {
    for (let n = this; n && n.nodeType === 1; n = n.parentNode) if (n.matches(selector)) return n;
    return null;
  }
  querySelectorAll(selector) {
    const chains = parseSelector(selector);
    const out = [];
    (function walk(node) {
      for (const child of node.children) {
        if (chains.some((chain) => matchChain(child, chain))) out.push(child);
        walk(child);
      }
    })(this);
    return out;
  }
  querySelector(selector) { return this.querySelectorAll(selector)[0] || null; }
}

for (const [prop, attr] of Object.entries(PROPERTY_ATTRS)) {
  Object.defineProperty(Element.prototype, prop, {
    get() { return this.getAttribute(attr) || ''; },
    set(v) { this.setAttribute(attr, String(v)); },
  });
}

function serialize(node) {
  if (node.nodeType === 3) return escapeText(node.data);
  let html = '<' + node.localName;
  for (const [k, v] of node.attributes) html += ' ' + k + '="' + String(v).replace(/&/g, '&amp;').replace(/"/g, '&quot;') + '"';
  if (VOID.has(node.localName)) return html + '>';
  return html + '>' + node.childNodes.map(serialize).join('') + '</' + node.localName + '>';
}

// ---- HTML parsing (well-formed output of markdown-it, WaveDrom and datasheet.js) ----

function parseInto(root, html) {
  const doc = root.ownerDocument;
  const stack = [root];
  const re = /<!--[\s\S]*?-->|<\/([\w:-]+)\s*>|<([\w:-]+)((?:\s+[^\s=/>]+(?:\s*=\s*(?:"[^"]*"|'[^']*'|[^\s>]+))?)*)\s*(\/?)>|[^<]+|</g;
  let m;
  while ((m = re.exec(html)) !== null) {
    const top = stack[stack.length - 1];
    if (m[0].startsWith('<!--')) continue;
    if (m[1]) {
      const tag = m[1].toLowerCase();
      const at = stack.map((n) => n.localName).lastIndexOf(tag);
      if (at > 0) stack.length = at;
    } else if (m[2]) {
      const el = new Element(doc, m[2]);
      const attrRe = /([^\s=/>]+)(?:\s*=\s*(?:"([^"]*)"|'([^']*)'|([^\s>]+)))?/g;
      let a;
      while ((a = attrRe.exec(m[3])) !== null) {
        const v = a[2] !== undefined ? a[2] : a[3] !== undefined ? a[3] : a[4] !== undefined ? a[4] : '';
        el.attributes.set(a[1], decode(v));
      }
      top.append(el);
      if (!m[4] && !VOID.has(el.localName)) stack.push(el);
    } else {
      top.append(decode(m[0]));
    }
  }
}

// ---- Selectors ----

function parseCompound(text) {
  const c = { tag: null, ids: [], classes: [], attrs: [], lastOfType: false };
  const re = /^([\w-]+|\*)|#([\w-]+)|\.([\w-]+)|\[([\w-]+)(?:="([^"]*)")?\]|:last-of-type/g;
  let m;
  let used = 0;
  while ((m = re.exec(text)) !== null && m[0]) {
    used += m[0].length;
    if (m[1]) c.tag = m[1] === '*' ? null : m[1].toLowerCase();
    else if (m[2]) c.ids.push(m[2]);
    else if (m[3]) c.classes.push(m[3]);
    else if (m[4]) c.attrs.push([m[4], m[5]]);
    else c.lastOfType = true;
  }
  if (used !== text.length) throw new Error('fake-dom: unsupported selector "' + text + '"');
  return c;
}

function parseSelector(selector) {
  return selector.split(',').map((part) => part.trim().split(/\s+/).map(parseCompound));
}

function matchCompound(el, c) {
  if (c.tag && el.localName !== c.tag) return false;
  if (c.ids.some((id) => el.id !== id)) return false;
  if (c.classes.some((k) => !el.classList.contains(k))) return false;
  if (c.attrs.some(([k, v]) => !el.hasAttribute(k) || (v !== undefined && el.getAttribute(k) !== v))) return false;
  if (c.lastOfType) {
    const same = el.parentNode.children.filter((n) => n.localName === el.localName);
    if (same[same.length - 1] !== el) return false;
  }
  return true;
}

function matchChain(el, chain) {
  if (!matchCompound(el, chain[chain.length - 1])) return false;
  let k = chain.length - 2;
  for (let n = el.parentNode; k >= 0 && n && n.nodeType === 1; n = n.parentNode) {
    if (matchCompound(n, chain[k])) k--;
  }
  return k < 0;
}

// ---- Document and window ----

function createDom() {
  const posted = { edit: [], cite: [] };
  const timers = [];
  const doc = {
    activeElement: null,
    title: '',
    createElement: (tag) => new Element(doc, tag),
    getElementById: (id) => doc.documentElement.querySelector('#' + id),
    querySelector: (s) => doc.documentElement.querySelector(s),
    querySelectorAll: (s) => doc.documentElement.querySelectorAll(s),
  };
  doc.documentElement = new Element(doc, 'html');
  doc.body = new Element(doc, 'body');
  doc.documentElement.append(doc.body);
  doc.body.innerHTML = '<div id="page"></div>';

  const window = {
    document: doc,
    Event,
    scrollY: 120,
    scrolledTo: [],
    scrollTo(x, y) { window.scrolledTo.push([x, y]); },
    setTimeout(fn, ms) { timers.push({ fn, ms }); return timers.length; },
    clearTimeout(id) { if (timers[id - 1]) timers[id - 1].fn = null; },
    webkit: { messageHandlers: {
      edit: { postMessage: (m) => posted.edit.push(JSON.parse(JSON.stringify(m))) },
      cite: { postMessage: (m) => posted.cite.push(JSON.parse(JSON.stringify(m))) },
    } },
  };
  window.window = window;
  const context = vm.createContext(window);

  return {
    window,
    document: doc,
    posted,
    Event,
    /** Runs a page script in the window under its real path (so coverage counts it). */
    load(file) { vm.runInContext(fs.readFileSync(file, 'utf8'), context, { filename: file }); },
    /** Runs the timers queued so far (and any they queue). */
    flush() {
      while (timers.some((t) => t.fn)) {
        for (const t of timers) { const fn = t.fn; t.fn = null; if (fn) fn(); }
      }
    },
  };
}

module.exports = { createDom, Event, decode };
