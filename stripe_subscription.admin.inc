<?php
/**
 * @file
 * Administrative callbacks for the stripe_subscription module.
 */

/**
 * Subscription Settings
 */
function stripe_subscription_admin_settings($form, &$form_state) {
  $secret = stripe_get_key("secret");
  $publishable = stripe_get_key("publishable");
  $error = NULL;
  if (empty($secret) || empty($publishable)) {
    $error = t("Please configure the stripe keys below to see your acccount information");
  }
  elseif (!$library = stripe_load_library()) {
    $error = t("Failed to load Stripe library.");
  }

  if (isset($error)) {
    $form['error_markup'] = array(
      '#markup' => '<p>' . $error . '</p>'
    );
    return $form;
  }

  $form['general'] = array(
    '#type' => 'fieldset',
    '#title' => t('General Settings')
  );

  $form['general']['stripe_subscription_coupons_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Coupon Support'),
    '#description' => t('When coupons are enabled, a textfield for a coupon code will be made available. Coupons are created in your Stripe control panel on stripe.com.'),
    '#default_value' => variable_get('stripe_subscription_coupons_enabled', 0)
  );

  $form['general']['stripe_subscription_address_fields_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Address Fields'),
    '#description' => t('When enabled, the subscription form will show additional fields for capturing the users address. This can help with validating their card details with Stripe.'),
    '#default_value' => variable_get('stripe_subscription_address_fields_enabled', 0)
  );

  $form['reg'] = array(
    '#type' => 'fieldset',
    '#title' => t('User Registration')
  );

  $form['reg']['stripe_subscription_reg_enable'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show subscription form on user registration form'),
    '#default_value' => variable_get('stripe_subscription_reg_enable', 0),
  );

  $form['reg']['stripe_subscription_reg_req'] = array(
    '#type' => 'checkbox',
    '#title' => t('Require a subscription when a user registers'),
    '#default_value' => variable_get('stripe_subscription_reg_req', 0),
  );

  $form['grade'] = array(
    '#type' => 'fieldset',
    '#title' => t('Upgrade/Downgrade Behavior (Experimental, do not use!)'),
    '#description' => t('This is only applicable to plans with the same term')
  );

  $form['grade']['stripe_subscription_upgrade_method'] = array(
    '#type' => 'radios',
    '#title' => t('Upgrade method'),
    '#description' => t('Select the model to use when a user upgrades their plan.'),
    '#options' => array(
      0 => t('Charge the full amount immediately and set new expiration date'),
      1 => t('Charge the pro-rated amount immediately and keep existing expiration date'),
      2 => t('Charge the pro-rated amount at the next renewal date')
    ),
    '#default_value' => variable_get('stripe_subscription_upgrade_method', 0)
  );

  $form['grade']['stripe_subscription_downgrade_method'] = array(
    '#type' => 'radios',
    '#title' => t('Downgrade method'),
    '#description' => t('Select the model to use when a user upgrades their plan.'),
    '#options' => array(
      0 => t('Charge the full amount immediately and set new expiration date'),
      2 => t('Charge the pro-rated amount at the next renewal date')
    ),
    '#default_value' => variable_get('stripe_subscription_downgrade_method', 0)
  );

  $form['notifications'] = array(
    '#type' => 'fieldset',
    '#title' => t('Notifications'),
  );

  $form['notifications']['stripe_subscribe_notify_created'] = array(
    '#type' => 'textarea',
    '#title' => t('Subscription Created'),
    '#default_value' => variable_get('stripe_subscribe_notify_created', '')
  );
  $form['notifications']['stripe_subscribe_notify_updated'] = array(
    '#type' => 'textarea',
    '#title' => t('Subscription Updated'),
    '#default_value' => variable_get('stripe_subscribe_notify_updated', '')
  );

  $form['notifications']['stripe_subscribe_notify_cancelled'] = array(
    '#type' => 'textarea',
    '#title' => t('Subscription Cancelled'),
    '#default_value' => variable_get('stripe_subscribe_notify_cancelled', '')
  );

  $form['notifications']['stripe_subscribe_notify_pmt_success'] = array(
    '#type' => 'textarea',
    '#title' => t('Payment Success'),
    '#default_value' => variable_get('stripe_subscribe_notify_pmt_success', '')
  );

  $form['notifications']['stripe_subscribe_notify_pmt_failed'] = array(
    '#type' => 'textarea',
    '#title' => t('Payment Failed'),
    '#default_value' => variable_get('stripe_subscribe_notify_pmt_failed', '')
  );

  $form['notifications']['stripe_subscribe_pmt_upcoming_days'] = array(
    '#type' => 'textfield',
    '#title' => t('Days before upcoming payment notification'),
    '#description' => t('Enter how many days before an upcoming payment you would like to send the <i>Upcoming Payment</i> notification. Enter <i>0</i> for no notification.'),
    '#default_value' => variable_get('stripe_subscribe_pmt_upcoming_days', '3'),
    '#size' => 5
  );

  $form['notifications']['stripe_subscribe_notify_pmt_upcoming'] = array(
    '#type' => 'textarea',
    '#title' => t('Upcoming Payment'),
    '#default_value' => variable_get('stripe_subscribe_notify_pmt_upcoming', '')
  );

  $form['notifications']['token_help'] = array(
    '#type' => 'fieldset',
    '#title' => t('Replacement Patterns'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  $form['notifications']['token_help']['content'] = array(
    '#type' => 'markup',
    '#token_types' => array('stripe-subscription'),
    '#theme' => 'token_tree'
  );

  return system_settings_form($form);
}

/**
 * Form validation for admin settings
 */
function stripe_subscription_admin_settings_validate(&$form, &$form_state) {
  if (!is_numeric($form_state['values']['stripe_subscribe_pmt_upcoming_days'])) {
    form_set_error('stripe_subscribe_pmt_upcoming_days', t('Days must be numeric'));
  }
}

/**
 * Plan manager form.
 * Draggable assignment architecture taken from the block module.
 *
 * @see template_preprocess_stripe_subscription_admin_plans()
 * @see stripe_subscription-admin-plan-form.tpl.php
 * @see theme_stripe_subscription_admin_plans()
 */
function stripe_subscription_admin_plans($form) {
  $form['#attached']['css'] = array(drupal_get_path('module', 'stripe_subscription') . '/stripe_subscription.css');
  $form['plans'] = array();
  $form['#tree'] = TRUE;

  $plans = _stripe_subscription_get_plans('all');

  $items = array(
    l('Add a plan group', 'admin/config/stripe/plan-groups/add'),
    l('Add a plan', 'admin/config/stripe/plans/add')
  );

  $action_links = theme('item_list', array('items' => $items, 'attributes' => array('class' => 'action-links')));

  $form['action_links'] = array(
    '#markup' => $action_links
  );

  if (count($plans) == 0) {
    $form['intro_copy'] = array('#markup' => 'No plans found.');
  }
  else {
    $plan_groups = db_select('stripe_plan_groups', 'pg')
      ->fields('pg', array('pgid', 'name', 'description'))
      ->execute()->fetchAllAssoc('pgid');

    $plan_group_opts = array();

    foreach ($plan_groups as $key => $pg) {
      $plan_group_opts[$key] = $pg->name;
      if ($key == 1) {
        $plan_groups[$key]->configure_link = '';
        $plan_groups[$key]->remove_link = '';
      }
      else {
        $plan_groups[$key]->configure_link = l('configure group', 'admin/config/stripe/plan-groups/' . $key);
        $plan_groups[$key]->remove_link = l('remove group', 'admin/config/stripe/plan-groups/' . $key . '/remove');
      }
    }

    $form['plan_groups'] = array(
      '#type' => 'value',
      '#value' => $plan_groups,
    );

    $weight_delta = round(count($plans) / 2);
    foreach ($plans as $plan) {
      $key = $plan->id;

      $form['plans'][$key]['weight'] = array(
        '#type' => 'weight',
        '#default_value' => isset($plan->weight) ? $plan->weight : NULL,
        '#delta' => $weight_delta,
        '#title_display' => 'invisible',
        '#title' => t('Weight for @plan plan', array('@plan' => $plan->name)),
      );
      $form['plans'][$key]['pgid'] = array(
        '#type' => 'select',
        '#default_value' => isset($plan->pgid) ? $plan->pgid : PLAN_GROUP_DEFAULT,
        '#empty_value' => PLAN_GROUP_DEFAULT,
        '#title_display' => 'invisible',
        '#title' => t('Group for @plan plan', array('@plan' => $plan->name)),
        '#options' => $plan_group_opts,
      );
      $form['plans'][$key]['id'] = array(
        '#markup' => $plan->id,
      );
      $form['plans'][$key]['name'] = array(
        '#markup' => $plan->name,
      );
      $form['plans'][$key]['amount'] = array(
        '#markup' => number_format($plan->amount/100, 2),
      );
      $form['plans'][$key]['currency'] = array(
        '#markup' => $plan->currency,
      );
      $form['plans'][$key]['interval'] = array(
        '#markup' => $plan->interval_count . ' ' . ucfirst($plan->interval),
      );
      $form['plans'][$key]['trial_period'] = array(
        '#markup' => $plan->trial_period_days,
      );
      $form['plans'][$key]['configure'] = array(
        '#type' => 'link',
        '#title' => t('configure'),
        '#href' => 'admin/config/stripe/plans/' . $plan->id,
      );
      $form['plans'][$key]['roles'] = array(
        '#type' => 'link',
        '#title' => t('Roles'),
        '#href' => 'admin/config/stripe/plans/' . $plan->id . '/roles',
      );
    }
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update Plans')
  );

  return $form;
}

/**
 * Processes variables for stripe_subscription-admin-plan-form.tpl.php.
 *
 * The $variables array contains the following arguments:
 * - $form
 *
 * @see stripe_subscription_admin_plans()
 * @see stripe_subscription-admin-plan-form.tpl.php
 * @see theme_stripe_subscription_admin_plans()
 */
function template_preprocess_stripe_subscription_admin_plans(&$variables) {

  $variables['action_links'] = drupal_render($variables['form']['action_links']);
  $variables['plan_groups'] = isset($variables['form']['plan_groups']) ? $variables['form']['plan_groups']['#value'] : array();

  foreach ($variables['plan_groups'] as $key => $value) {
    // Initialize an empty array for the plan group.
    $variables['plans'][$key] = array();
  }

  // Initialize disabled blocks array.
  $variables['plans'][PLAN_GROUP_DEFAULT] = array();

  // Add each plan in the form to the appropriate place in the plan group hierarchy.
  foreach (element_children($variables['form']['plans']) as $i) {
    $plan = &$variables['form']['plans'][$i];

    // Fetch the plan group for the current plan.
    $pgid = (isset($plan['pgid']['#default_value']) ? $plan['pgid']['#default_value'] : PLAN_GROUP_DEFAULT);

    // Set special classes needed for table drag and drop.
    $plan['pgid']['#attributes']['class'] = array('pgid-select', 'pg-' . $pgid);
    $plan['weight']['#attributes']['class'] = array('plan-weight', 'plan-weight-' . $pgid);

    $variables['plans'][$pgid][$i] = new stdClass();
    $variables['plans'][$pgid][$i]->row_class = !empty($plan['#attributes']['class']) ? implode(' ', $plan['#attributes']['class']) : '';
    $variables['plans'][$pgid][$i]->block_modified = !empty($plan['#attributes']['class']) && in_array('plan-modified', $plan['#attributes']['class']);
    $variables['plans'][$pgid][$i]->id = drupal_render($plan['id']);
    $variables['plans'][$pgid][$i]->name = drupal_render($plan['name']);
    $variables['plans'][$pgid][$i]->amount = drupal_render($plan['amount']);
    $variables['plans'][$pgid][$i]->currency = drupal_render($plan['currency']);
    $variables['plans'][$pgid][$i]->interval = drupal_render($plan['interval']);
    $variables['plans'][$pgid][$i]->trial_period = drupal_render($plan['trial_period']);
    $variables['plans'][$pgid][$i]->pgid_select = drupal_render($plan['pgid']);
    $variables['plans'][$pgid][$i]->weight_select = drupal_render($plan['weight']);
    $variables['plans'][$pgid][$i]->configure_link = drupal_render($plan['configure']);
    $variables['plans'][$pgid][$i]->roles_link = drupal_render($plan['roles']);
  }

  $variables['form_submit'] = drupal_render_children($variables['form']);
}

/**
 * Implements hook_submit().
 */
function stripe_subscription_admin_plans_submit(&$form, &$form_state) {
  $livemode = stripe_customer_livemode();
  db_delete('stripe_plan_pg')
    ->condition('livemode', $livemode)
    ->execute();
  foreach ($form_state['values']['plans'] as $plan_id => $values) {
    db_insert('stripe_plan_pg')
      ->fields(array('pgid' => $values['pgid'], 'plan_id' => $plan_id,  'weight' => $values['weight'], 'livemode' => $livemode))
      ->execute();
  }
  drupal_set_message('Plans have been updated.');
}

/**
 * Add/Edit Plans
 */
function stripe_subscription_admin_plans_edit($form, $form_state, $plan_id = NULL) {
  $plan = NULL;
  $secret = stripe_get_key("secret");
  $publishable = stripe_get_key("publishable");
  $error = NULL;
  if (empty($secret) || empty($publishable)) {
    $error = t("Please configure the stripe keys below to see your acccount information");
  }
  elseif (!$library = stripe_load_library()) {
    $error = t("Failed to load Stripe library.");
  }

  $account = \Stripe\Account::retrieve();

  if (isset($plan_id) && $plan_id != 'add') {
    try {
      $plan = \Stripe\Plan::retrieve($plan_id);
      drupal_set_title('Edit Stripe Plan: ' . $plan->name);
      drupal_set_message('You may only update the name of a plan.', 'warning');
    }
    catch (Exception $e) {
      $error = t('Plan not found.');
    }
  }

  if (isset($error)) {
    drupal_set_message($error);
    drupal_goto('admin/config/stripe/plans');
  }

  $form['existing'] = array(
    '#type' => 'hidden',
    '#value' => isset($plan) ? 1 : 0
  );

  $form['stripe'] = array(
    '#type' => 'fieldset',
    '#title' => t('Stripe Plan Parameters')
  );

  if (isset($plan)) {
    $form['plan_id'] = array(
      '#type' => 'hidden',
      '#value' => $plan->id
    );
    $form['stripe']['plan_id_copy'] = array(
      '#markup' => '<div><label>ID </label>' . $plan->id . '</div>'
    );
  }
  else {
    $form['stripe']['plan_id'] = array(
      '#type' => 'textfield',
      '#title' => t('ID'),
      '#description' => t('Unique string of your choice that will be used to identify this plan when subscribing a customer. This could be an identifier like "gold" or a primary key from your own database.'),
      '#required' => TRUE,
      '#default_value' => NULL,
      '#size' => 20
    );
  }

  $form['stripe']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Name of the plan, to be displayed on invoices and in the web interface.'),
    '#required' => TRUE,
    '#default_value' => isset($plan) ? $plan->name : NULL,
    '#size' => 40
  );

  $form['stripe']['amount'] = array(
    '#type' => 'textfield',
    '#title' => t('Amount (in cents)'),
    '#description' => t('A positive integer in cents (or 0 for a free plan) representing how much to charge (on a recurring basis)'),
    '#required' => TRUE,
    '#default_value' => isset($plan) ? $plan->amount : NULL,
    '#disabled' => isset($plan) ? TRUE : FALSE,
    '#size' => 20
  );

  // Get the supported currencies for the country this Stripe Account is located in.
  $country_spec = \Stripe\CountrySpec::retrieve($account->country);
  $supported_currencies = $country_spec->supported_payment_currencies;

  $form['stripe']['currency'] = array(
    '#type' => 'select',
    '#title' => t('Currency'),
    '#description' => t('Currency for this plan. See all supported currencies in the <a href="@link">Settings page</a>.',
      array('@link' => 'admin/config/stripe/settings')),
    '#options' => drupal_map_assoc($supported_currencies),
    '#default_value' => isset($plan) ? $plan->currency : $account->default_currency,
    '#required' => TRUE,
    '#disabled' => isset($plan) ? TRUE : FALSE,
  );

  $form['stripe']['interval'] = array(
    '#type' => 'radios',
    '#title' => t('Billing Interval'),
    '#description' => t('Specifies billing frequency.'),
    '#options' => array('day' => t('Day'), 'week' => t('Week'), 'month' => t('Month'), 'year' => t('Year')),
    '#default_value' => isset($plan) ? $plan->interval : 'month',
    '#disabled' => isset($plan) ? TRUE : FALSE,
    '#required' => TRUE
  );

  $form['stripe']['interval_count'] = array(
    '#type' => 'textfield',
    '#title' => t('Interval Count'),
    '#description' => t('The number of the unit specified in the interval parameter. For example, you could specify an interval_count of 3 and an interval of \'month\' for quarterly billing (every 3 months).'),
    '#default_value' => isset($plan) ? $plan->interval_count : 1,
    '#disabled' => isset($plan) ? TRUE : FALSE,
    '#size' => 5
  );

  $form['stripe']['trial_period_days'] = array(
    '#type' => 'textfield',
    '#title' => t('Trial Period (in days)'),
    '#description' => t('Specifies a trial period in (an integer number of) days. If you include a trial period, the customer won\'t be billed for the first time until the trial period ends. If the customer cancels before the trial period is over, she\'ll never be billed at all.'),
    '#default_value' => isset($plan) ? $plan->trial_period_days : NULL,
    '#disabled' => isset($plan) ? TRUE : FALSE,
    '#size' => 5
  );

  $form['stripe']['statement_descriptor'] = array(
    '#type' => 'textfield',
    '#title' => t('(Optional) Statement Descriptor'),
    '#description' => t("An arbitrary string to be displayed on your customer’s credit card statement. Limit of 22 characters."),
    '#default_value' => isset($plan) ? $plan->statement_descriptor : '',
    '#disabled' => isset($plan) ? TRUE : FALSE,
    '#size' => 25,
    '#maxlength' => '22',
  );

  $submit_text = isset($plan) ? t('Update Plan') : t('Create Plan');
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $submit_text
  );

  return $form;
}

/**
 * Implements hook_validate()
 */
function stripe_subscription_admin_plans_edit_validate($form, &$form_state) {
  if (!preg_match('/^\d+$/', $form_state['values']['amount']) || $form_state['values']['amount'] < 0) {
    form_set_error('amount', t('Amount must be a positive integer. (The price in cents)'));
  }

  stripe_load_library();
  $plan_data = array(
    'amount' => $form_state['values']['amount'],
    'interval' => $form_state['values']['interval'],
    'interval_count' => $form_state['values']['interval_count'],
    'trial_period_days' => !empty($form_state['values']['trial_period_days']) ? $form_state['values']['trial_period_days'] : NULL,
    'name' => $form_state['values']['name'],
    'currency' => $form_state['values']['currency'],
    'id' => $form_state['values']['plan_id'],
    'statement_descriptor' => check_plain($form_state['values']['statement_descriptor']),
  );

  try {
    if ($form_state['values']['existing'] == 1) {
      $p = \Stripe\Plan::retrieve($plan_data['id']);
      if ($p->name != $plan_data['name']) {
        $p->name = $plan_data['name'];
        $p->save();
      }
    }
    else {
      \Stripe\Plan::create($plan_data);
    }
  }
  catch (Exception $e) {
    //watchdog('stripe', 'Stripe_Plan::create Exception: %str', array('%str' => $e->getMessage()));
    $field = '';
    $message = $e->getMessage();
    $trace = $e->getTrace();
    if (is_array($trace)) {
      $last = array_shift($trace);
      if (isset($last['args'][2])) {
        $field = isset($last['args'][2]['error']['param']) ? $last['args'][2]['error']['param'] : '';
        $message = $last['args'][2]['error']['message'];
      }
    }
    form_set_error($field, $message);
  }
}

/**
 * Implements hook_submit().
 */
function stripe_subscription_admin_plans_edit_submit($form, &$form_state) {
  if ($form_state['values']['existing'] == 1) {
    drupal_set_message('Stripe plan updated successfully.');
  }
  else {
    drupal_set_message('Stripe plan added successfully.');
  }
  $form_state['redirect'] = 'admin/config/stripe/plans';
}

/**
 * Plan group add form.
 */
function stripe_subscription_admin_plan_group_edit($form, &$form_state, $pgid = NULL) {
  $pg = NULL;
  if (isset($pgid) && is_numeric($pgid)) {
    $result = db_select('stripe_plan_groups', 'pg')
      ->fields('pg', array('name', 'description'))
      ->condition('pg.pgid', $pgid)
      ->execute();
    $pg = $result->fetchObject();
  }

  if (isset($pg)) {
    $form['pgid'] = array(
      '#type' => 'hidden',
      '#value' => $pgid
    );
  }
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Group name'),
    '#default_value' => isset($pg) ? $pg->name : NULL,
    '#required' => TRUE
  );
  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#default_value' => isset($pg) ? $pg->description : NULL,
    '#required' => TRUE
  );

  $str = isset($pg) ? t('Update Group') : t('Add Group');
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $str
  );

  return $form;
}

/**
 * Implements hook_submit().
 */
function stripe_subscription_admin_plan_group_edit_submit(&$form, &$form_state) {
  $fields = array(
    'name' => $form_state['values']['name'],
    'description' => $form_state['values']['description']
  );
  if (isset($form_state['values']['pgid'])) {
    db_update('stripe_plan_groups')
      ->fields($fields)
      ->condition('pgid', $form_state['values']['pgid'])
      ->execute();
    drupal_set_message('Plan group has been updated.');
  }
  else {
    db_insert('stripe_plan_groups')
      ->fields($fields)
      ->execute();
    drupal_set_message('Plan group has been added.');
  }

  $form_state['redirect'] = 'admin/config/stripe/plans';
}

/**
 * Plan group remove form
 */
function stripe_subscription_admin_plan_group_remove($form, &$form_state, $pgid) {

  $form['pgid'] = array(
    '#type' => 'hidden',
    '#value' => $pgid
  );

  $plan_group = db_select('stripe_plan_groups', 'pg')
    ->fields('pg', array('name'))
    ->condition('pgid', $pgid)
    ->execute()
    ->fetchField();

  $form['copy'] = array(
    '#markup' => '<p>' . t('Are you sure you would like to remove plan group: %plan_group?', array('%plan_group' => $plan_group->name)) . '</p>'
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Remove Plan Group'),
    '#prefix' => l('Cancel', 'admin/config/stripe/plans')
  );

  return $form;
}

/**
 * Implements hook_submit().
 */
function stripe_subscription_admin_plan_group_remove_submit(&$form, &$form_state) {
  $livemode = stripe_customer_livemode();

  db_update('stripe_plan_pg')
    ->fields(array('pgid' => 1))
    ->condition('pgid', $form_state['values']['pgid'])
    ->condition('livemode', $livemode)
    ->execute();
  db_delete('stripe_plan_groups')
    ->condition('pgid', $form_state['values']['pgid'])
    ->execute();
  drupal_set_message('Plan group has been removed.');
  $form_state['redirect'] = 'admin/config/stripe/plans';
}

/**
 * Role assignment form
 */
function stripe_subscription_admin_plans_roles($form, &$form_state, $plan_id) {
  $library = stripe_load_library();
  $plan = NULL;
  $secret = stripe_get_key("secret");
  $publishable = stripe_get_key("publishable");
  $error = NULL;
  if (empty($secret) || empty($publishable)) {
    $error = t("Please configure the stripe keys below to see your acccount information");
  }
  elseif (!$library = stripe_load_library()) {
    $error = t("Failed to load Stripe library.");
  }

  if (!isset($error)) {
    try {
      $plan = \Stripe\Plan::retrieve($plan_id);
      drupal_set_title('Stripe Plan Roles: ' . $plan->name);
    }
    catch (Exception $e) {
      $error = t('Pan not found.');
    }
  }
  if (isset($error)) {
    drupal_set_message($error, 'error', 'error');
    drupal_goto('admin/config/stripe/plans');
    return;
  }

  $roles = user_roles(TRUE);
  unset($roles[2]);

  if (count($roles) === 0) {
    drupal_set_message(t('No custom roles found.'), 'warning');
    $form['copy'] = array(
      '#markup' => '<p>' . t('You must add custom roles in order to assign a role to a subscription.') . '</p>'
    );
    return $form;
  }

  $form['plan_id'] = array(
    '#type' => 'hidden',
    '#value' => $plan->id
  );

  $form['roles_subscribe'] = array(
    '#type' => 'fieldset',
    '#title' => t('Subscribe'),
    '#description' => t('These roles assignment settings will be set when a user subscribes to this plan.')
  );

  $livemode = stripe_customer_livemode();
  $rids = db_select('stripe_plan_roles', 's')
    ->fields('s', array('rid'))
    ->condition('s.event', 'subscribe')
    ->condition('s.action', 'add')
    ->condition('s.livemode', $livemode)
    ->condition('s.plan_id', $plan->id)
    ->execute()->fetchCol();
  $form['roles_subscribe']['subscribe_add'] = array(
    '#type' => 'select',
    '#title' => t('Roles to Add'),
    '#options' => $roles,
    '#default_value' => drupal_map_assoc($rids),
    '#multiple' => TRUE
  );

  $rids = db_select('stripe_plan_roles', 's')
    ->fields('s', array('rid'))
    ->condition('s.event', 'subscribe')
    ->condition('s.action', 'remove')
    ->condition('s.livemode', $livemode)
    ->condition('s.plan_id', $plan->id)
    ->execute()->fetchCol();

  $form['roles_subscribe']['subscribe_remove'] = array(
    '#type' => 'select',
    '#title' => t('Roles to Remove'),
    '#options' => $roles,
    '#default_value' => drupal_map_assoc($rids),
    '#multiple' => TRUE
  );

  $form['roles_unsubscribe'] = array(
    '#type' => 'fieldset',
    '#title' => t('Un-Subscribe'),
    '#description' => t('These roles assignment settings will be set when a user un-subscribes from this plan.')
  );

  $rids = db_select('stripe_plan_roles', 's')
    ->fields('s', array('rid'))
    ->condition('s.event', 'unsubscribe')
    ->condition('s.action', 'add')
    ->condition('s.livemode', $livemode)
    ->condition('s.plan_id', $plan->id)
    ->execute()->fetchCol();
  $form['roles_unsubscribe']['unsubscribe_add'] = array(
    '#type' => 'select',
    '#title' => t('Roles to Add'),
    '#options' => $roles,
    '#default_value' => drupal_map_assoc($rids),
    '#multiple' => TRUE
  );

  $rids = db_select('stripe_plan_roles', 's')
    ->fields('s', array('rid'))
    ->condition('s.event', 'unsubscribe')
    ->condition('s.action', 'remove')
    ->condition('s.livemode', $livemode)
    ->condition('s.plan_id', $plan->id)
    ->execute()->fetchCol();

  $form['roles_unsubscribe']['unsubscribe_remove'] = array(
    '#type' => 'select',
    '#title' => t('Roles to Remove'),
    '#options' => $roles,
    '#default_value' => drupal_map_assoc($rids),
    '#multiple' => TRUE
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update Role Assignments')
  );

  return $form;
}

/**
 * Implements hook_submit().
 */
function stripe_subscription_admin_plans_roles_submit($form, &$form_state) {
  $livemode = stripe_customer_livemode();

  // Remove role records for this plan
  db_delete('stripe_plan_roles')
    ->condition('plan_id', $form_state['values']['plan_id'])
    ->condition('livemode', $livemode)
    ->execute();

  // Add role records for this plan
  if (is_array($form_state['values']['subscribe_add'])) {
    foreach ($form_state['values']['subscribe_add'] as $rid){
      db_insert('stripe_plan_roles')
        ->fields(array(
          'plan_id' => $form_state['values']['plan_id'],
          'livemode' => $livemode,
          'event' => 'subscribe',
          'action' => 'add',
          'rid' => $rid
        ))
        ->execute();
    }
  }
  if (is_array($form_state['values']['subscribe_remove'])) {
    foreach ($form_state['values']['subscribe_remove'] as $rid){
      db_insert('stripe_plan_roles')
        ->fields(array(
        'plan_id' => $form_state['values']['plan_id'],
        'livemode' => $livemode,
        'event' => 'subscribe',
        'action' => 'remove',
        'rid' => $rid
        ))
        ->execute();
    }
  }
  if (is_array($form_state['values']['unsubscribe_add'])) {
    foreach ($form_state['values']['unsubscribe_add'] as $rid){
      db_insert('stripe_plan_roles')
        ->fields(array(
        'plan_id' => $form_state['values']['plan_id'],
        'livemode' => $livemode,
        'event' => 'unsubscribe',
        'action' => 'add',
        'rid' => $rid
        ))
        ->execute();
    }
  }
  if (is_array($form_state['values']['unsubscribe_remove'])) {
    foreach ($form_state['values']['unsubscribe_remove'] as $rid){
      db_insert('stripe_plan_roles')
        ->fields(array(
        'plan_id' => $form_state['values']['plan_id'],
        'livemode' => $livemode,
        'event' => 'unsubscribe',
        'action' => 'remove',
        'rid' => $rid
        ))
        ->execute();
    }
  }

  drupal_set_message('Plan roles updated successfully.');
  $form_state['redirect'] = 'admin/config/stripe/plans';
}

/**
 * Form to delete local test data.
 */
function stripe_subscription_admin_delete_test_data($form, $form_state) {

  $form['copy'] = array(
    '#markup' => '<p>Are you sure you would like to delete the local test data? Note that this does not delete the test data on Stripe. You must also delete test data over there.</p>'
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Yes, Delete Local Test Data')
  );

  return $form;
}

function stripe_subscription_admin_delete_test_data_submit($form, &$form_state) {
  db_query("DELETE FROM {stripe_subscriptions} WHERE livemode = 0");
  db_query("DELETE FROM {stripe_subscriptions} WHERE livemode = 0");
  drupal_set_message('Local test data deleted successfully.');
}
