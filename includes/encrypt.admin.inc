<?php

/**
 * @file
 * This file holds the functions for the Encrypt admin settings.
 *
 * @ingroup encrypt
 */

/**
 * Menu callback; displays the list of profiles.
 */
function encrypt_configs_list() {
  $configs = encrypt_get_configs();
  $methods = encrypt_get_encryption_methods();
  $providers = encrypt_get_key_providers();
  $default_config = config_get('encrypt.settings', 'default_config');

  $header = array(
    t('Name'),
    t('Encryption Method'),
    t('Key Provider'),
    t('Status'),
    t('Operations'),
  );
  $rows = array();

  foreach ($configs as $config) {
    $label = $config['label'];
    $name = $config['name'];
    $description = $config['description'];
    $method = $config['method'];
    $provider = $config['provider'];

    $config_url_str = str_replace('_', '-', $name);

    $variables = array(
      'label' => $label,
      'name' => $name,
      'description' => $description,
    );

    // Set the name column.
    $row = array(theme('encrypt_configs_list_description', $variables));

    // Set the encryption method column.
    $row[] = array('data' => $methods[$method]['title']);

    // Set the key provider column.
    $row[] = array('data' => $providers[$provider]['title']);

    // Set the status column.
    $status = array();
    $status[] = ($config['enabled']) ? t('Enabled') : t('Disabled');
    if ($default_config == $config['name']) {
      $status[] = t('Default');
    }
    $row[] = array('data' => implode(', ', $status));

    $links = array();
    $links['configure'] = array(
      'title' => t('Configure'),
      'href' => ENCRYPT_MENU_PATH . '/edit/' . $config_url_str,
    );
    $links['delete'] = array(
      'title' => t('Delete'),
      'href' => ENCRYPT_MENU_PATH . '/delete/' . $config_url_str,
    );
    // Set the make default column if this is not already the default.
    if ($default_config != $name) {
      $links['make_default'] = array(
        'title' => t('Make default'),
        'href' => ENCRYPT_MENU_PATH . '/default/' . $config_url_str,
      );
    }
    $operations = array(
      '#type' => 'operations',
      '#links' => $links,
    );
    $row[] = array('data' => $operations);

    $rows[] = $row;
  }

  $build['encrypt_configs_list_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No encryption profiles are available. <a href="@link">Add a profile</a>.', array('@link' => url(ENCRYPT_MENU_PATH . '/add'))),
  );

  return $build;
}

/**
 * Form constructor for the profile edit form.
 *
 * @param array $config
 *   (optional) An array representing the profile, when editing an
 *   existing profile.
 *
 * @see encrypt_config_form_validate()
 * @see encrypt_config_form_submit()
 * @ingroup forms
 */
function encrypt_config_form($form, &$form_state, array $config = NULL) {
  $default_config = config_get('encrypt.settings', 'default_config');

  // Clear the plugin cache on the first page load (but not on AJAX refreshes).
  if (!isset($form_state['values'])) {
    _encrypt_clear_plugin_cache();
  }

  // Retrieve all methods and providers.
  $methods = encrypt_get_encryption_methods();
  $providers = encrypt_get_key_providers();

  // Make sure all plugin files are included (for form validation, etc.)
  // @todo: There has to be a better/faster/cleaner way to do this.
  foreach ($providers as $provider) {
    $form_state['build_info']['files']["encrypt:plugins:{$provider['name']}"] = $provider['path'] . '/' . $provider['file'];
  }

  // Create a list of method titles to be used for radio buttons.
  // Only display a deprecated method if it's the currently selected
  // method for this profile. List it at the bottom.
  $method_options = array();
  foreach ($methods as $id => $method) {
    if (!$method['deprecated']) {
      $method_options[$id] = $method['title'];
    }
  }
  foreach ($methods as $id => $method) {
    if ($method['deprecated'] && isset($config['method']) && $id == $config['method']) {
      $method_options[$id] = $method['title'] . ' <span class="encrypt-warning">' . t('(Deprecated)') . '</span>';
    }
  }

  // Create a list of provider titles to be used for radio buttons.
  // Only display a deprecated provider if it's the currently selected
  // provider for this profile. List it at the bottom.
  $provider_options = array();
  foreach ($providers as $id => $provider) {
    if (!$provider['deprecated']) {
      $provider_options[$id] = $provider['title'];
    }
  }
  foreach ($providers as $id => $provider) {
    if ($provider['deprecated'] && isset($config['provider']) && $id == $config['provider']) {
      $provider_options[$id] = $provider['title'] . ' <span class="encrypt-warning">' . t('(Deprecated)') . '</span>';
    }
  }

  $form = array();

  $form['#attached'] = array(
    'css' => array(
      backdrop_get_path('module', 'encrypt') . '/css/encrypt.css',
    ),
  );

  $form['label'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#default_value' => isset($config['label']) ? $config['label'] : NULL,
    '#description' => t('The human-readable name of the profile.'),
    '#required' => TRUE,
    '#size' => 30,
  );
  $form['name'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($config['name']) ? $config['name'] : NULL,
    '#maxlength' => 32,
    '#disabled' => isset($config['name']),
    '#machine_name' => array(
      'exists' => 'encrypt_config_load',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for the profile. It must only contain lowercase letters, numbers, and underscores.'),
  );
  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textarea',
    '#default_value' => isset($config['description']) ? $config['description'] : NULL,
    '#description' => t('A short description of the profile.'),
  );

  $form['settings'] = array(
    '#type' => 'vertical_tabs',
    '#attached' => array(
      'js' => array(backdrop_get_path('module', 'encrypt') . '/js/encrypt_admin.js'),
    ),
  );

  $form['general_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('General settings'),
    '#collapsible' => TRUE,
    '#group' => 'settings',
  );
  $form['general_settings']['enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enabled'),
    '#default_value' => isset($config['enabled']) ? $config['enabled'] : NULL,
    '#description' => t('If checked, this profile will be available for encryption. The default profile must be enabled.'),
  );

  // If this is the default profile, disable the enabled checkbox.
  if (isset($config['name']) && $config['name'] == $default_config) {
    $form['general_settings']['enabled']['#disabled'] = TRUE;
  }

  $form['method_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Encryption method settings'),
    '#collapsible' => TRUE,
    '#group' => 'settings',
  );
  $form['method_settings']['encrypt_encryption_method'] = array(
    '#type' => 'radios',
    '#title' => t('Encryption Method'),
    '#description' => t('Define the default encryption method for the site. Since encryption methods are stored with the data, this can be changed even after you have stored encrypted data.'),
    '#required' => TRUE,
    '#options' => $method_options,
    '#default_value' => isset($config['method']) ? $config['method'] : NULL,
    '#ajax' => array(
      'method' => 'replace',
      'callback' => 'encrypt_encryption_methods_additional_settings_ajax',
      'wrapper' => 'encrypt-encryption-methods-additional-settings',
    ),
  );
  // Disable any method with dependency errors.
  _encrypt_admin_form_add_options($methods, $form['method_settings']['encrypt_encryption_method']);

  $form['method_settings']['method_settings_wrapper'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="encrypt-encryption-methods-additional-settings">',
    '#suffix' => '</div>',
  );

  if (isset($form_state['values']['encrypt_encryption_method'])) {
    $method = $form_state['values']['encrypt_encryption_method'];
  }
  elseif (isset($config['method'])) {
    $method = $config['method'];
  }
  else {
    $method = NULL;
  }
  $method_settings = isset($config['method_settings']) ? $config['method_settings'] : array();

  if ($method) {
    if ($method_settings_form = plugin_manager_get_function($methods[$method], 'settings form')) {
      $form['method_settings']['method_settings_wrapper']['method_settings'] = array(
        '#type' => 'fieldset',
        '#title' => t('Additional Encryption Method Settings'),
      );

      $form['method_settings']['method_settings_wrapper']['method_settings']['encrypt_encryption_methods_' . $method . '_settings'] = array('#tree' => TRUE) + call_user_func($method_settings_form, $method_settings);
    }
  }

  $form['provider_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Key provider settings'),
    '#collapsible' => TRUE,
    '#group' => 'settings',
  );
  $form['provider_settings']['encrypt_key_provider'] = array(
    '#type' => 'radios',
    '#title' => t('Key Provider'),
    '#description' => t('Select the provider from which an encryption key will be retrieved. Check the encryption method for requirements before creating the key. NOTE: Once this is set, it is <strong>not</strong> a good idea to change it. All of your encrypted data may be lost if the encryption key changes.'),
    '#required' => TRUE,
    '#options' => $provider_options,
    '#default_value' => isset($config['provider']) ? $config['provider'] : NULL,
    '#ajax' => array(
      'method' => 'replace',
      'callback' => 'encrypt_key_providers_additional_settings_ajax',
      'wrapper' => 'encrypt-key-providers-additional-settings',
    ),
  );
  // Disable any method with dependency errors.
  _encrypt_admin_form_add_options($providers, $form['provider_settings']['encrypt_key_provider']);

  $form['provider_settings']['key_settings_wrapper'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="encrypt-key-providers-additional-settings">',
    '#suffix' => '</div>',
  );

  if (isset($form_state['values']['encrypt_key_provider'])) {
    $provider = $form_state['values']['encrypt_key_provider'];
  }
  elseif (isset($config['provider'])) {
    $provider = $config['provider'];
  }
  else {
    $provider = NULL;
  }
  $provider_settings = isset($config['provider_settings']) ? $config['provider_settings'] : array();

  if ($provider) {
    if ($provider_settings_form = plugin_manager_get_function($providers[$provider], 'settings form')) {
      $form['provider_settings']['key_settings_wrapper']['key_settings'] = array(
        '#type' => 'fieldset',
        '#title' => t('Additional Key Provider Settings'),
      );

      $form['provider_settings']['key_settings_wrapper']['key_settings']['encrypt_key_providers_' . $provider . '_settings'] = array('#tree' => TRUE) + call_user_func($provider_settings_form, $provider_settings);
    }
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save profile'),
    '#submit' => array('encrypt_config_form_submit'),
    '#weight' => 40,
  );
  if (isset($config['name'])) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete profile'),
      '#submit' => array('encrypt_config_form_delete_submit'),
      '#limit_validation_errors' => array(),
      '#weight' => 45,
    );
  }

  return $form;
}

/**
 * Form validation handler for encrypt_config_form().
 */
function encrypt_config_form_validate($form, &$form_state) {
  // Don't allow the default profile to be disabled.
  $default_config = config_get('encrypt.settings', 'default_config');

  if ($default_config == $form_state['values']['name'] && !$form_state['values']['enabled']) {
    form_set_error('general_settings][enabled', t('The default profile must be enabled.'));
  }
}

/**
 * Form submission handler for encrypt_config_form().
 */
function encrypt_config_form_submit($form, &$form_state) {
  $method = encrypt_get_encryption_method($form_state['values']['encrypt_encryption_method']);
  $key_provider = encrypt_get_key_provider($form_state['values']['encrypt_key_provider']);

  foreach (array($method, $key_provider) as $plugin) {
    if ($submit_callback = plugin_manager_get_function($plugin, 'submit callback')) {
      // Create a copy so that the plugin callback cannot change the
      // form state.
      $form_state_copy = $form_state;
      call_user_func($submit_callback, $form, $form_state_copy);
    }
  }

  $fields = array(
    'name' => (string) $form_state['values']['name'],
    'label' => (string) $form_state['values']['label'],
    'description' => (string) $form_state['values']['description'],
    'method' => (string) $form_state['values']['encrypt_encryption_method'],
    'provider' => (string) $form_state['values']['encrypt_key_provider'],
    'enabled' => (int) $form_state['values']['enabled'],
  );

  if (empty($form_state['values']['encrypt_encryption_methods_' . $fields['method'] . '_settings'])) {
    $fields['method_settings'] = '';
  }
  else {
    $fields['method_settings'] = $form_state['values']['encrypt_encryption_methods_' . $fields['method'] . '_settings'];
  }

  if (empty($form_state['values']['encrypt_key_providers_' . $fields['provider'] . '_settings'])) {
    $fields['provider_settings'] = '';
  }
  else {
    $fields['provider_settings'] = $form_state['values']['encrypt_key_providers_' . $fields['provider'] . '_settings'];
  }

  encrypt_save_config($fields);

  $form_state['redirect'] = ENCRYPT_MENU_PATH . '/list';
}

/**
 * Form submission handler for encrypt_config_form().
 *
 * Handles the 'Delete' button on the encryption profile form.
 */
function encrypt_config_form_delete_submit($form, &$form_state) {
  $form_state['redirect'] = ENCRYPT_MENU_PATH . '/delete/' . str_replace('_', '-', $form['name']['#default_value']);
}

/**
 * Menu callback to delete an encryption profile.
 *
 * @ingroup forms
 */
function encrypt_config_delete_confirm($form, &$form_state, $config) {
  $default_config = config_get('encrypt.settings', 'default_config');

  if ($default_config == $config['name']) {
    backdrop_set_message(t('The default profile cannot be deleted.'), 'error');
    backdrop_goto(ENCRYPT_MENU_PATH);
  }

  $form['name'] = array('#type' => 'value', '#value' => $config['name']);
  $form['label'] = array('#type' => 'value', '#value' => $config['label']);

  $message = t('Are you sure you want to delete the profile %label?', array('%label' => $config['label']));
  $caption = '<p>' . t('This action cannot be undone.') . '</p>';

  return confirm_form($form, filter_xss_admin($message), ENCRYPT_MENU_PATH, filter_xss_admin($caption), t('Delete'));
}

/**
 * Submit handler for encrypt_config_delete_confirm.
 *
 * @see encrypt_config_delete_confirm()
 */
function encrypt_config_delete_confirm_submit($form, &$form_state) {
  $config = config('encrypt.profile.' . $form_state['values']['name']);
  $config->delete();

  $t_args = array('%label' => $form_state['values']['label']);
  backdrop_set_message(t('The profile %label has been deleted.', $t_args));
  watchdog('encrypt', 'Deleted encryption profile %label.', $t_args, WATCHDOG_NOTICE);

  $form_state['redirect'] = ENCRYPT_MENU_PATH;
}

/**
 * Menu callback to make a profile the default.
 */
function encrypt_config_make_default($config) {
  $config_settings = config('encrypt.settings');
  $config_settings->set('default_config', $config['name']);
  $config_settings->save();

  $default_config = $config_settings->get('default_config');
  $t_args = array('%label' => $config['label']);
  if ($default_config == $config['name']) {
    // If the profile is not enabled and it's the new default, enable it.
    if (!$config['enabled']) {
      config_set('encrypt.profile.' . $config['name'], 'enabled', 1);
      backdrop_set_message(t('The profile %label has been enabled.', $t_args));
      watchdog('node', 'Enabled encryption profile %label.', $t_args, WATCHDOG_NOTICE);
    }

    backdrop_set_message(t('The profile %label has been made the default.', $t_args));
    watchdog('encrypt', 'Made encryption profile %label the default.', $t_args, WATCHDOG_NOTICE);
  }
  else {
    backdrop_set_message(t('The profile %label could not be made the default.', $t_args), 'error');
    watchdog('encrypt', 'Error when trying to make encryption profile %label the default.', $t_args, WATCHDOG_ERROR);
  }

  backdrop_goto(ENCRYPT_MENU_PATH);
}

/**
 * Callback for AJAX form re-rendering for method additional settings.
 */
function encrypt_encryption_methods_additional_settings_ajax($form, $form_state) {
  return $form['method_settings']['method_settings_wrapper'];
}

/**
 * Callback for AJAX form re-rendering for provider additional settings.
 */
function encrypt_key_providers_additional_settings_ajax($form, $form_state) {
  return $form['provider_settings']['key_settings_wrapper'];
}

/**
 * Add other elements to forms.
 */
function _encrypt_admin_form_add_options($items, &$element) {
  foreach ($items as $id => $item) {
    $element[$id] = array(
      '#description' => $item['description'],
      '#disabled' => !empty($item['dependency errors']),
    );

    // Add a list of dependency errors (if there are any).
    if (!empty($item['dependency errors'])) {
      $element[$id]['#description'] .= theme('item_list', array(
        'items' => $item['dependency errors'],
        'attributes' => array('class' => 'encrypt-dependency-errors'),
      ));
    }
  }
}

/**
 * Old settings callback.
 *
 * This form is no longer used, so if a user reaches it, display a notice
 * that an update is necessary.
 */
function encrypt_admin_settings($form, &$form_state) {
  _encrypt_display_update_message();
}

/**
 * Returns HTML for a profile description.
 *
 * @param array $variables
 *   An associative array containing:
 *   - label: The human-readable label of the profile.
 *   - name: The machine name of the profile.
 *   - description: A brief description of the profile.
 *
 * @ingroup themeable
 */
function theme_encrypt_configs_list_description(array $variables) {
  $label = $variables['label'];
  $name = $variables['name'];
  $description = $variables['description'];

  $output = check_plain($label);
  $output .= ' <small>' . t('(Machine name: @name)', array('@name' => $name)) . '</small>';
  $output .= '<div class="description">' . filter_xss_admin($description) . '</div>';

  return $output;
}
