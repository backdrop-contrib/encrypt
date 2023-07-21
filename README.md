Encrypt
=======

This module is an API that other modules can use to encrypt data. It
doesn't provide any user-facing features of its own, aside from
administration pages to manage configuration.

The encrypt module Provides an API for two-way encryption. Backdrop has no
native way to do two-way encryption. PHP's ability to do two-way encryption is a
little more involved than most people care to get into.  This module provides an
easy way to encrypt() and decrypt().

Installation
------------

Install this module using the official Backdrop CMS instructions at
<https://backdropcms.org/guide/modules>.

Configuration
-------------

**Encrypt** allows multiple configurations to be managed within a Backdrop site.
Each configuration contains an encryption method and a key provider, along with
any additional settings that the method or provider requires.

API
---

The Encrypt API module provides a low-level system for two-way encryption. While
it provides developers and users with a few basic encryption methods and key
providers, developers may wish to extend either of these areas to provide more
robust options. Both encryption methods and key providers are implemented as
plugins, so providing your own encryption methods and key providers is really
easy and pretty well-documented.

The default encryption methods and key providers supplied by the Encrypt API
module are also implemented as plugins, so for good examples of how to implement
your own, looking at the code would be a good place to start. You can find them
in the Encrypt module's directory, in the *plugins* folder.

Encryption methods
------------------

To provide an encryption method to the Encrypt API module, you need to do a
couple things:

Implement `hook_plugin_manager_plugin_directory` in your module. This simply tells
plugin_manager where to look for your module's plugins. Here is an example from
Encrypt's own plugins.

```php
function encrypt_plugin_manager_plugin_directory($module, $plugin) {
  if ($module == 'encrypt') {
    return 'plugins/' . $plugin;
  }
}
```

So, with this implementation, plugin_manager will look for encryption method
plugins in `plugins/encryption_methods` and key provider plugins in
`plugins/key_providers`. You can load plugins however you'd like, but this is
sort of a plugin_manager standard.

Create your plugin file. It should be a file called `YOUR_PLUGIN.inc` (where
`YOUR_PLUGIN` is some machine-readable name for your plugin) inside the folder
 you declared above.

In your plugin file, declare your plugin. Here is an example from Encrypt's
"AES (OpenSSL) + HMAC-SHA256" encryption method.

```php
$plugin = encrypt_openssl_encrypt_encryption_methods();

/**

* Implements MODULE_FILENAME_encrypt_encryption_methods().
 */
function encrypt_openssl_encrypt_encryption_methods() {
  return array(
    'title' => t('AES (OpenSSL) + HMAC-SHA256'),
    'description' => t('Uses AES-128-CBC via OpenSSL along with HMAC-SHA256.'),
    'encrypt callback' => '_encrypt_encryption_methods_openssl',
    'dependency callback' => '_encrypt_openssl_extension_is_present',
  );
}
```

Available options for the `$plugin` array include:

**title**
(Required). The human-readable name for your encryption method. This will
appear on the Encrypt admin page.

**description**
(Required). A brief description of your encryption method. Also appears in
 smaller text on the Encrypt admin page.

**encrypt callback**
(Required). This is the name of a function that you define in your plugin
file. Encrypt will use this function to encrypt and decrypt text for your
encryption method.

**dependency callback**
(Optional). The name of a function in your plugin file that declares whether
or not an encryption method's dependencies have been met. The function should
return an array of error messages (if there are any) or an empty array or FALSE
if all dependencies are met. For example:

```php
/**
 * Callback to see if the Mcrypt library is present.
 */
function _encrypt_mcrypt_extension_is_present() {
  $errors = array();

  if (!function_exists('mcrypt_encrypt')) {
    $errors[] = t('Mcrypt library not installed.');
  }

  return $errors;
}
```

**submit callback**
(Optional). The name of a function that will be called when the encrypt
settings form is saved. This allows plugins to perform additional actions when
settings are saved. The function should take the $form and $form_state as
arguments, just like most other form submit handlers. See the file key provider
plugin for an example.

Create your encrypt/decrypt method. This is the function you declared in
**encrypt callback** above. The function will have the following signature:

```php
/**
 * @param $op
 *  The operation currently being performed. 'encrypt' for encrypting, 'decrypt' for decrypting
 *
 * @param $text
 *  The string to be encrypted or decrypted.
 *
 * @param $key
 *  The encryption key to use for encrypting. Provided by the active key provider.
 *
 * @param $options
 *  An array of additional parameters.
 *
 * @return
 *  The processed (either encrypted or decrypted, depending on $op) string.
 */
function your_encryption_callback($op = 'encrypt', $text = '', $key, $options = array()) {
  // Do stuff.
}
```

All encryption method plugins are cached by plugin_manager, so you may have to
clear Backdrop's cache for new plugins or changes to existing plugins to appear.

Key providers
-------------

Implementing key providers is, programmatically, very similar to defining
encryption methods.

The paramters of a key provider plugin are as follows:

**title**
(Required). The human-readable name for your key provider. This will
appear on the Encrypt admin page.

**description**
(Required). A brief description of your key provider. Also appears in
smaller text on the Encrypt admin page.

**key callback**
(Required). This is the name of a function that you define in your plugin
file. This function will be responsible for return an encryption key of some
kind.

**dependencies**
(Optional). The name of a function in your plugin file that declares whether
or not a key provider's dependencies have been met. The function should return
an array of error messages (if there are any) or an empty array or FALSE if all
dependencies are met. For example:

```php
/**
 * Callback to see if the Mcrypt library is present.
 */
function _encrypt_mcrypt_extension_is_present() {
  $errors = array();

  if (!function_exists('mcrypt_encrypt')) {
    $errors[] = t('Mcrypt library not installed.');
  }

  return $errors;
}
```

**submit callback**
(Optional). The name of a function that will be called when the encrypt
settings form is saved. This allows plugins to perform additional actions when
settings are saved. The function should take the $form and $form_state as
arguments, just like most other form submit handlers. See the file key provider
plugin for an example.

**static key**
(Optional). A boolean value indicating if the key can be stored as a static
variable, so that it only needs to be retrieved once per page request. Set this
to FALSE if the key provider returns a different key based on a value that is
specific to a particular item, such as a node ID or a field's machine name.
Defaults to TRUE.

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.

Maintainers
-----------

* Herb v/d Dool <https://github.com/herbdool>

Credits
-------

Drupal version currently maintained by:

* <https://www.drupal.org/u/rlhawk>
* <https://www.drupal.org/u/greggles>
* <https://www.drupal.org/u/nerdstein>
* <https://www.drupal.org/u/theunraveler>
