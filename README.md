Encrypt
=======

This module is an API that other modules can use to encrypt data. It
doesn't provide any user-facing features of its own, aside from
administration pages to manage configuration.

The encrypt module Provides an API for two-way encryption. Backdrop has no native way to do two-way encryption. PHP's ability to do two-way encryption is a little more involved than most people care to get into.  This module provides an easy way to encrypt() and decrypt().

Installation
------------

Install this module using the official Backdrop CMS instructions at <https://backdropcms.org/guide/modules>.

Configuration
-------------

**Encrypt** allows multiple configurations to be managed within a
Backdrop site. Each configuration contains an encryption method and a
key provider, along with any additional settings that the method or
provider requires.

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.

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
