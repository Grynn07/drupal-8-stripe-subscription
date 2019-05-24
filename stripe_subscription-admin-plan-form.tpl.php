<?php

/**
 * @file
 * Default theme implementation to configure plans with plan groups.
 *
 * Available variables:
 * - $action_links: Add plan and Add plan group links
 * - $plan_groups: An array of plan groups. Keyed by plan group id with the title as value.
 * - $plans: An array of blocks keyed by plan group then delta.
 *
 * Each $block_listing[$region] contains an array of blocks for that region.
 *
 * Each $data in $block_listing[$region] contains:
 * - $data->region_title: Region title for the listed block.
 * - $data->block_title: Block title.
 * - $data->region_select: Drop-down menu for assigning a region.
 * - $data->weight_select: Drop-down menu for setting weights.
 * - $data->configure_link: Block configuration link.
 * - $data->delete_link: For deleting user added blocks.
 *
 * @see template_preprocess_block_admin_display_form()
 * @see theme_block_admin_display()
 *
 * @ingroup themeable
 */
?>
<?php
// Add table javascript.
drupal_add_js('misc/tableheader.js');
drupal_add_js(drupal_get_path('module', 'stripe_subscription') . '/stripe_subscription.js');
foreach ($plan_groups as $pgid => $pg) {
  drupal_add_tabledrag('plans', 'match', 'sibling', 'pgid-select', 'pg-' . $pgid, NULL, FALSE);
  drupal_add_tabledrag('plans', 'order', 'sibling', 'plan-weight', 'plan-weight-' . $pgid);
}
?>
<?php print $action_links; ?>

<table id="plans" class="sticky-enabled">
  <thead>
  <tr>
    <th><?php print t('Plan ID'); ?></th>
    <th><?php print t('Name'); ?></th>
    <th><?php print t('Group'); ?></th>
    <th><?php print t('Currency'); ?></th>
    <th><?php print t('Amount'); ?></th>
    <th><?php print t('Interval'); ?></th>
    <th><?php print t('Trial Period (days)'); ?></th>
    <th><?php print t('Weight'); ?></th>
    <th colspan="2"><?php print t('Operations'); ?></th>
  </tr>
  </thead>
  <tbody>
  <?php $row = 0; ?>
  <?php foreach ($plan_groups as $pgid => $pg): ?>
    <tr class="pg-title pg-title-<?php print $pgid?>">
      <td colspan="8">Group: <?php print $pg->name; ?></td>
      <td><?php print $pg->configure_link; ?></td>
      <td><?php print $pg->remove_link; ?></td>
    </tr>
    <tr class="pg-message pg-<?php print $pgid?>-message <?php print empty($plans[$pgid]) ? 'pg-empty' : 'pg-populated'; ?>">
      <td colspan="9"><em><?php print t('No plans in this group'); ?></em></td>
    </tr>
    <?php foreach ($plans[$pgid] as $delta => $data): ?>
      <tr class="draggable <?php print $row % 2 == 0 ? 'odd' : 'even'; ?><?php print $data->row_class ? ' ' . $data->row_class : ''; ?>">
        <td class="block"><?php print $data->id; ?></td>
        <td><?php print $data->name; ?></td>
        <td><?php print $data->pgid_select; ?></td>
        <td><?php print $data->currency; ?></td>
        <td><?php print $data->amount; ?></td>
        <td><?php print $data->interval; ?></td>
        <td><?php print $data->trial_period; ?></td>
        <td><?php print $data->weight_select; ?></td>
        <td><?php print $data->configure_link; ?></td>
        <td><?php print $data->roles_link; ?></td>
      </tr>
      <?php $row++; ?>
    <?php endforeach; ?>
  <?php endforeach; ?>
  </tbody>
</table>

<?php print $form_submit; ?>
