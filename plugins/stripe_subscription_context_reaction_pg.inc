<?php

/**
 * Expose blocks as context reactions.
 */
class stripe_subscription_context_reaction_pg extends context_reaction {

  /**
   * Options form.
   */
  function options_form($context) {
    $values = $this->fetch_from_context($context);

    $plan_groups = db_select('stripe_plan_groups', 'pg')
      ->fields('pg', array('pgid', 'name'))
      ->execute()->fetchAllKeyed();
    $form['pgid'] = array(
      '#type' => 'select',
      '#title' => t('Plan Group'),
      '#options' => $plan_groups,
      '#default_value' => isset($values['pgid']) ? $values['pgid'] : NULL
    );


    return $form;
  }

  /**
   * Set 'section_title', and 'section_subtitle' if not set
   */
  function execute() {
    $contexts = context_active_contexts();
    foreach ($contexts as $context) {
      if (!empty($context->reactions['plan_group']) && isset($context->reactions['plan_group']['pgid'])) {
        // calls get_plan_groups with the pgid to set the static var
        $plans = _stripe_subscription_get_plans($context->reactions['plan_group']['pgid']);
      }
    }
  }
}