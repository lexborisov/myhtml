// Usage: include a link like `<a href="https://github.com/whatwg/{my-repo}/issues/new">file an issue</a>`, or give it
// `id="file-issue-link"` instead. Then include this script with
// `<script src="https://resources.whatwg.org/file-issue.js" defer></script>`. Style the element using the selector
// `.selected-text-file-an-issue`. If you don't want the script to inject styles, use a `data-no-style` attribute on
// the script element.
//
// If you don't have a file an issue link on your spec (e.g. for a spec split into multiple documents), you can use
// a `data-file-issue-url=""` attribute on the `<script>` tag.

(function () {
  'use strict';
  var thisScript = document.currentScript;

  var originalFilingUrl = getOriginalFilingUrl();
  var prevSelectionText = null;

  var fileLink = document.createElement('a');
  fileLink.href = getFilingUrl(originalFilingUrl, window.getSelection());
  fileLink.accessKey = '1';
  fileLink.className = 'selected-text-file-an-issue';
  fileLink.textContent = 'File an issue about the selected text';

  if (!thisScript.hasAttribute('data-no-style')) {
    // Consider moving this to a .css file in the future, perhaps bikeshed.css once all specs have moved to Bikeshed.
    var style = document.createElement('style');
    style.textContent = '.selected-text-file-an-issue { position: fixed; bottom: 0; right: 0; ' +
                        'background: rgba(255, 255, 255, 0.8); font-size: smaller; padding: 4px 10px; z-index: 1; ' +
                        'text-decoration: underline; }';
    document.head.appendChild(style);
  }

  document.body.appendChild(fileLink);

  window.addEventListener('mouseup', handleInteraction);
  window.addEventListener('keydown', handleInteraction);

  function handleInteraction(event) {
    var selection = window.getSelection();
    if (selection.toString() === prevSelectionText) {
      return;
    }

    fileLink.href = getFilingUrl(originalFilingUrl, selection, event.target);
  }

  function getOriginalFilingUrl() {
    var link = document.querySelector('#file-issue-link, a[href$="/issues/new"]');
    if (link) {
      return link.href;
    }

    var dataAttr = thisScript.getAttribute("data-file-issue-url");
    if (dataAttr) {
      return dataAttr;
    }

    throw new Error('No "file an issue" link found and no data-file-issue-url attribute present on the script');
  }

  function getFilingUrl(originalFilingUrl, selection, startNode) {
    var bugData = getBugData(selection, startNode);
    return originalFilingUrl + '?title=' + encodeURIComponent(bugData.title) + '&body=' +
           encodeURIComponent(bugData.body);
  }

  function getBugData(selection, startNode) {
    var selectionText = selection.toString();
    var url = getUrlToReport(selection, startNode);

    return {
      title: getTitle(selectionText),
      body: getBody(url, selectionText)
    };
  }
  
  function escapeGFM(text) {
    return text.replace(/&/g, '&amp;') // HTML
               .replace(/</g, '&lt;') // HTML
               .replace(/>/g, '&gt;') // blockquote
               .replace(/([:@=])/g, '$1\u200b') // emoji, @mention, headings
               .replace(/([\\`\*_\{\}\[\]\(\)#\+\-\.!\~\|])/g, '\\$1'); // other formatting
  }

  function getBody(url, selectionText) {
    var quotedText = selectionText;
    if (quotedText.length > 1000) {
      quotedText = quotedText.substring(0, 997) + '...';
    }

    quotedText = escapeGFM(quotedText).replace(/\r/g, '').replace(/\n/g, '\n> ');
    if (quotedText.length > 0) {
      quotedText = '> ' + quotedText;
    }

    return url + '\n\n' + quotedText;
  }

  function getTitle(selectionText) {
    var title = selectionText.toString().replace(/\n/g, ' ');
    if (title.length > 50) {
      title = title.substring(0, 47) + '...';
    }

    if (title.length > 0) {
      title = '"' + title + '"';
    }

    return title;
  }

  function getUrlToReport(selection, startNode) {
    var url = window.location.href;

    var node = getBestNodeToReport(selection, startNode);
    if (node) {
      url = url.split('#')[0] + '#' + node.id;
    }

    return url;
  }

  function getBestNodeToReport(selection, fallback) {
    var node = fallback;

    if (selection.anchorNode) {
      node = selection.anchorNode;

      if (selection.focusNode && selection.focusNode.compareDocumentPosition) {
        var compare = selection.focusNode.compareDocumentPosition(selection.anchorNode);
        if (compare & Node.DOCUMENT_POSITION_FOLLOWING || compare & Node.DOCUMENT_POSITION_CONTAINED_BY) {
          node = selection.focusNode;
        }
      }
    }

    while (node && !node.id) {
      node = node.previousSibling || node.parentNode;
    }

    return node;
  }
}());
