<?php
/**
 * Create terms for Opus Roles
 *
 * @param
 *  array of Opus Roles to create if they do not exist already
 */
function os2intra_user_import_create_opus_roles($opus_roles) {
  if (empty($opus_roles)) {
    return;
  }

  $vocabulary = taxonomy_vocabulary_machine_name_load('os2intra_bi_opus_roles');
  if ($vocabulary) {
    os2intra_user_import_save_log('', 'Opus: creation new opus role started');
    $existing_roles = array();

    $terms = entity_load('taxonomy_term', FALSE, array('vid' => $vocabulary->vid));

    foreach ($terms as $term) {
      $term = entity_metadata_wrapper('taxonomy_term', $term->tid);
      $existing_roles[] = $term->os2intra_bi_machine_name->value();
    }

    foreach ($opus_roles as $opus_role) {
      if (!in_array(strtolower(trim($opus_role)), $existing_roles)) {
        $newterm = new stdClass();
        $newterm->name = $opus_role;
        $newterm->vid = $vocabulary->vid;
        $newterm->parent = 0;
        $newterm->os2intra_bi_machine_name[LANGUAGE_NONE][0]['value'] = strtolower(trim($opus_role));
        taxonomy_term_save($newterm);
      }
    }
    os2intra_user_import_save_log('', 'Opus: creation new opus role finished');
  }
}
