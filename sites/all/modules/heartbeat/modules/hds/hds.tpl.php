<?php

/**
 * @file
 * Template file for one row, rendered by heartbeat
 *
 * @var
 * - $message : after it was parsed by heartbeat (grouped)
 * - $row : All parts that are separately themeable
 * -   $row->message : string activity message
 * -   $row->time_info : information about the time of activity
 * -   $row->class : extra classes to use on the row
 * -   $row->attachments : attachment on the message id (of the grouped message)
 *
 * @remarks
 *   beat-item-<uaid> is necessairy. The beat item id is used to toggle
 *   visibility of the "number more" messages when grouping exceeded the
 *   maximum allowed grouped property.
 */
?>
<div class="heartbeat-message-block <?php print $message->message_id . ' ' . $zebra; ?>">

  <div class="beat-item" id="beat-item-<?php print $message->uaid ?>">

    <?php print $content; ?>

  </div>

  <?php if (count($message->uaids) > 0) :?>
  <div class="beat-item <?php print $message->classes ?> beat-item-ungrouped" id="beat-item-<?php print $message->uaid ?>-ungrouped" style="display: none;">
  <?php foreach ($message->additions->source as $ungrouped_message) { ?>
    <?php print $ungrouped_message; ?><br />
  <?php } ?>
    <div class="heartbeat-buttons">
    <?php print l(t('Back'), drupal_get_destination(), array('attributes' => array('onclick' => 'javascript:Drupal.heartbeat.splitGroupedMessage(' . $message->uaid .', null); return false;')))?>
    </div>
    <br class="clearfix" />
  </div>
  <?php endif; ?>

</div>