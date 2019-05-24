(function ($) {

  /**
   * Move a plan in the plans table from one group to another via select list.
   *
   * This behavior is dependent on the tableDrag behavior, since it uses the
   * objects initialized in that behavior to update the row.
   */
  Drupal.behaviors.stripeSubscriptionPlanDrag = {
    attach: function (context, settings) {
      // tableDrag is required and we should be on the plans admin page.
      if (typeof Drupal.tableDrag == 'undefined' || typeof Drupal.tableDrag.plans == 'undefined') {
        return;
      }

      var table = $('table#plans');
      var tableDrag = Drupal.tableDrag.plans; // Get the plans tableDrag object.

      // Add a handler for when a row is swapped, update empty regions.
      tableDrag.row.prototype.onSwap = function (swappedRow) {
        checkEmptyPlanGroups(table, this);
      };

      // A custom message for the blocks page specifically.
      Drupal.theme.tableDragChangedWarning = function () {
        return '<div class="messages warning">' + Drupal.theme('tableDragChangedMarker') + ' ' + Drupal.t('The changes to these groups will not be saved until the <em>Save plans</em> button is clicked.') + '</div>';
      };

      // Add a handler so when a row is dropped, update fields dropped into new regions.
      tableDrag.onDrop = function () {
        dragObject = this;
        // Use "region-message" row instead of "region" row because
        // "region-{region_name}-message" is less prone to regexp match errors.
        var pgRow = $(dragObject.rowObject.element).prevAll('tr.pg-message').get(0);
        var pgName = pgRow.className.replace(/([^ ]+[ ]+)*pg-([^ ]+)-message([ ]+[^ ]+)*/, '$2');
        var pgField = $('select.pgid-select', dragObject.rowObject.element);
        // Check whether the newly picked group is available for this plan.
        if ($('option[value=' + pgName + ']', pgField).length == 0) {
          // If not, alert the user and keep the plan in its old region setting.
          alert(Drupal.t('The plan cannot be placed in this group.'));
          // Simulate that there was a selected element change, so the plan is put
          // back to from where the user tried to drag it.
          pgField.change();
        }
        else if ($(dragObject.rowObject.element).prev('tr').is('.pg-message')) {
          var weightField = $('select.plan-weight', dragObject.rowObject.element);
          var oldPgName = weightField[0].className.replace(/([^ ]+[ ]+)*plan-weight-([^ ]+)([ ]+[^ ]+)*/, '$2');
          if (!pgField.is('.pg-' + pgName)) {
            pgField.removeClass('pg-' + oldPgName).addClass('pg-' + pgName);
            weightField.removeClass('plan-weight-' + oldPgName).addClass('plan-weight-' + pgName);
            pgField.val(pgName);
          }
        }
      };

      // Add the behavior to each region select list.
      $('select.pg-select', context).once('pg-select', function () {
        $(this).change(function (event) {
          // Make our new row and select field.
          var row = $(this).closest('tr');
          var select = $(this);
          tableDrag.rowObject = new tableDrag.row(row);

          // Find the correct plan group and insert the row as the last in the region.
          table.find('.pg-' + select[0].value + '-message').nextUntil('.pg-message').last().before(row);

          // Modify empty plan groups with added or removed fields.
          checkEmptyPlanGroups(table, row);
          // Remove focus from selectbox.
          select.get(0).blur();
        });
      });

      var checkEmptyPlanGroups = function (table, rowObject) {
        $('tr.pg-message', table).each(function () {
          // If the dragged row is in this region, but above the message row, swap it down one space.
          if ($(this).prev('tr').get(0) == rowObject.element) {
            // Prevent a recursion problem when using the keyboard to move rows up.
            if ((rowObject.method != 'keyboard' || rowObject.direction == 'down')) {
              rowObject.swap('after', this);
            }
          }
          // This plan group has become empty.
          if ($(this).next('tr').is(':not(.draggable)') || $(this).next('tr').length == 0) {
            $(this).removeClass('pg-populated').addClass('pg-empty');
          }
          // This plan group has become populated.
          else if ($(this).is('.pg-empty')) {
            $(this).removeClass('pg-empty').addClass('pg-populated');
          }
        });
      };
    }
  };

})(jQuery);
