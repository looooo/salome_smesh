(function() {
  'use strict';

  // Parses versions in URL segments like:

  var all_languages = {
      'en': 'English',
      'fr': 'French',
      'ja': 'Japanese',
  };


  function build_language_select(current_language) {
    var buf = ['<select>'];

    $.each(all_languages, function(language, title) {
      if (language == current_language)
        buf.push('<option value="' + language + '" selected="selected">' +
                 all_languages[current_language] + '</option>');
      else
        buf.push('<option value="' + language + '">' + title + '</option>');
    });
    buf.push('</select>');
    return buf.join('');
  }

  function navigate_to_first_existing(urls) {
    // Navigate to the first existing URL in urls.
    var url = urls.shift();
    if (urls.length == 0) {
      window.location.href = url;
      return;
    }
    $.ajax({
      url: url,
      success: function() {
        window.location.href = url;
      },
      error: function() {
        navigate_to_first_existing(urls);
      }
    });
  }

  function on_language_switch() {
    var selected_language = $(this).children('option:selected').attr('value') + '/';
    var url = window.location.href;
    var current_language = language_segment_from_url(url);
    var current_version = version_segment_in_url(url);
    if (selected_language == 'en/') // Special 'default' case for english.
      selected_language = '';
    var new_url = url.replace('.org/' + current_language + current_version,
                              '.org/' + selected_language + current_version);
    if (new_url != url) {
      navigate_to_first_existing([
        new_url,
        'https://docs.python.org/'
      ]);
    }
  }

  // Returns the path segment of the language as a string, like 'fr/'
  // or '' if not found.
  function language_segment_from_url(url) {
    var language_regexp = '\.org/([a-z]{2}(?:-[a-z]{2})?/)';
    var match = url.match(language_regexp);
    if (match !== null)
        return match[1];
    return '';
  }

  $(document).ready(function() {
    var release = DOCUMENTATION_OPTIONS.VERSION;
    var language_segment = language_segment_from_url(window.location.href);
    var current_language = language_segment.replace(/\/+$/g, '') || 'en';

    var language_select = build_language_select(current_language);

    $('.language_switcher_placeholder').html(language_select);
    $('.language_switcher_placeholder select').bind('change', on_language_switch);
  });
})();
