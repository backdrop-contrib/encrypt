<?php

/**
 * @file
 * Plugin definition for the Backdrop Private Key key provider.
 *
 * THIS PROVIDER IS DEPRECATED AND SHOULD NOT BE USED FOR ENCRYPTION.
 */

$plugin = encrypt_backdrop_private_key_encrypt_key_providers();

/**
 * Implements MODULE_FILENAME_encrypt_key_providers().
 */
function encrypt_backdrop_private_key_encrypt_key_providers() {
  return array(
    'title' => t('Backdrop Private Key'),
    'description' => t("Use Backdrop's private key from the database."),
    'key callback' => 'encrypt_get_backdrop_private_key',
    'settings form' => NULL,
    'static key' => TRUE,
    'deprecated' => TRUE,
  );
}

/**
 * Callback method to return Backdrop's private key.
 */
function encrypt_get_backdrop_private_key() {
  return backdrop_get_private_key();
}
