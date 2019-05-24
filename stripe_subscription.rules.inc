<?php
/**
 * Implements hook_rules_event_info().
 *
 * @ingroup rules
 */
function stripe_subscription_rules_event_info() {
  // @todo: group these by the object being sent by the webhook?
  // @todo: Can we use $event->object->object for this purpose?
  return array(
    'stripe_subscription_event' => array(
      'label' => t('A Subscription webhook was processed'),
      'help' => t('A Stripe Subscription webhook or event was triggered.'),
      'module' => 'stripe_subscription',
      'group' => 'Stripe Subscription',
      'variables' => array(
        'event_type' => array('type' => 'text', 'label' => t('The event type')),
        'user' => array(
          'type' => 'user',
          'label' => t('User'),
          'description' => t('The associated Drupal user.'),
        ),
        'subscription' => array('type' => 'stripe-subscription', 'label' => t('The subscription')),
      ),
      'callbacks' => array(
        //'execute' => 'stripe_subscription_event_'. $event_type;
        // @todo: Use a build callback here.
      ),
    ),
    'stripe_subscription_new' => array(
      'label' => t('A new subscription was created'),
      'help' => t('A customer just became a subscriber for the first time.'),
      'module' => 'stripe_subscription',
      'group' => 'Stripe Subscription',
      'variables' => array(
        'user' => array(
          'type' => 'user',
          'label' => t('User'),
          'description' => t('The associated Drupal user.'),
        ),
        'subscription' => array('type' => 'stripe-subscription', 'label' => t('The subscription')),
      ),
    ),
    'stripe_subscription_updated' => array(
      'label' => t('An existing subscription was updated'),
      'help' => t('A customer just updated their subscription details.'),
      'module' => 'stripe_subscription',
      'group' => 'Stripe Subscription',
      'variables' => array(
        'user' => array(
          'type' => 'user',
          'label' => t('User'),
          'description' => t('The associated Drupal user.'),
        ),
        'subscription' => array('type' => 'stripe-subscription', 'label' => t('The subscription')),
        'previous_plan' => array('type' => 'stripe-subscription', 'label' => t('The subscription')),
      ),
    ),
    'stripe_subscription_payment_updated' => array(
      'label' => t("A subscriber payment method was updated"),
      'help' => t('A customer just updated their payment info for a subscription'),
      'module' => 'stripe_subscription',
      'group' => 'Stripe Subscription',
      'variables' => array(
        'user' => array(
          'type' => 'user',
          'label' => t('User'),
          'description' => t('The associated Drupal user.'),
        ),
        'subscription' => array('type' => 'stripe-subscription', 'label' => t('The subscription')),
      ),
    ),
  );
}

/**
 * Implementation hook_rules_data_info().
 * @ingroup rules
 */
function stripe_subscription_rules_data_info() {
  return array(
    'stripe-subscription' => array(
      'label' => t('Stripe subscription'),
      'property info' => _stripe_subscription_rules_data_properties(),
    ),
  );
}

/**
 * Provide property info for the Stripe Subscription rules data.
 */
function _stripe_subscription_rules_data_properties() {
  return array(
    'customer_id' => array(
      'label' => t('Customer'),
      'type' => 'text',
      'description' => t('The Stripe Customer associated with a subscription'),
    ),
    'subscription_id' => array(
      'label' => t('Subscription ID'),
      'type' => 'text',
      'description' => t('The unique ID of this subscription.'),
    ),
    'plan_id' => array(
      'label' => t('Plan ID'),
      'type' => 'text',
      'description' => t('ID of the Stripe Plan the user is subscribed to'),
    ),
  );
}

/**
 * Implements hook_rules_condition_info().
 * @ingroup rules
 */
function stripe_subscription_rules_condition_info() {
  $conditions = array();
  $conditions['stripe_subscription_webhook_action'] = array(
    'label' => t("Subscription Webhook event has type"),
    'group' => t('Stripe Subscription'),
    'parameter' => array(
      'event_type' => array(
        'type' => 'text',
        'label' => t('Event type'),
        'options list' => 'stripe_subscription_event_types',
        'multiple' => FALSE,
        'widget' => 'select',
      ),
    ),
  );

  $conditions['stripe_subscription_user_is_subscribed'] = array(
    'label' => t("User is subscribed to a Plan"),
    'group' => t('Stripe Subscription'),
    'parameter' => array(
      'user' => array(
        'type' => 'user',
        'label' => t('User'),
      ),
      'plan' => array(
        'type' => 'text',
        'label' => t('Available Stripe Plans'),
        'options list' => 'stripe_subscription_plans',
      ),
    ),
  );

  return $conditions;
}

/**
 * Rules condition callback.
 *
 * Returns whether a subscription webhook type is of the configured types.
 */
function stripe_subscription_webhook_action($event_type, $params) {
  $type = $event_type;
  return $type == $params['event_type'];
}

