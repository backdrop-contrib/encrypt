/**
 * @file
 * Javascript behaviors for the Encrypt module.
 */

(function ($) {
  'use strict';
  Backdrop.behaviors.encryptFieldsetSummaries = {
    attach: function (context) {
      // Provide the vertical tab summaries.
      $('fieldset#edit-method-settings', context).backdropSetSummary(function (context) {
        var vals = [];
        var label = $('#edit-encrypt-encryption-method input:checked', context).next('label');
        if (label.text()) {
          vals.push(Backdrop.checkPlain(label.text()));
        }
        else {
          vals.push(Backdrop.t('No method chosen'));
        }
        return vals.join(', ');
      });
      $('fieldset#edit-provider-settings', context).backdropSetSummary(function (context) {
        var vals = [];
        var label = $('#edit-encrypt-key-provider input:checked', context).next('label');
        if (label.text()) {
          vals.push(Backdrop.checkPlain(label.text()));
        }
        else {
          vals.push(Backdrop.t('No provider chosen'));
        }
        return vals.join(', ');
      });
    }
  };

})(jQuery);
