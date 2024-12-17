<?php
/**
 * Convert numeric values into integers, else return the string as utf8.
 */
function _os2intra_user_import_sanitize_int($str) {
  if (is_numeric($str)) {
    return (int) $str;
  }
  return (mb_detect_encoding($str, "UTF-8, ISO-8859-1", true) != "UTF-8") ? utf8_encode($str) : $str;
}

/**
 * Replacing special characters in the string.
 */
function _os2intra_user_import_replace_special_chars($str) {
  // Mapping of characters to their text replacements.
  $charMap = [
    ',' => '_',
    '\\' => '_',
    '/' => '_',
    ':' => '_',
    '*' => '_',
    '?' => '_',
    '"' => '_',
    '<' => '_',
    '>' => '_',
    '|' => '_',
    ' ' => '_',
    '-' => '_',
    '.' => '_',
    '(' => '_',
    ')' => '_',
    '&' => 'og'
  ];

  // Replace characters based on the mapping
  return strtr($str, $charMap);
}

/**
 * Helper for user title
 */
function os2intra_user_import_user_title($job_description) {
  $term = reset(taxonomy_get_term_by_name($job_description));

  // Create new taxonomy term
  if (!is_object($term)) {
    $term = new stdClass();

    $term->name = $job_description;
    $term->vid = variable_get('os2intra_job_title_vocabulary_vid');

    taxonomy_term_save($term);
  }

  return $term->tid;
}

/**
 * Searches for terms by department id and resturns an EntityFieldQuery result.
 */
function os2intra_user_import_query_term_by_department($department_id) {
  $query = new EntityFieldQuery;
  $query->entityCondition('entity_type', 'taxonomy_term');
  $query->fieldCondition('field_os2intra_department_id', 'value', $department_id, '=');
  $result = $query->execute();

  if ($result) {
    $term = taxonomy_term_load(key($result['taxonomy_term']));
  }
  // Returning the array if exists and FALSE if not.
  if ($term) {
    return $result;
  } else {
    return FALSE;
  }
}

/**
 * Get department NID from department ID.
 */
function _os2intra_user_import_get_department_nid($department) {
  $query = new EntityFieldQuery;
  $query->entityCondition('entity_type', 'taxonomy_term');
  $query->fieldCondition('field_os2intra_department_id', 'value', $department, '=');
  $result = $query->execute();

  if (count($result['taxonomy_term']) > 0) {
    $tid = key($result['taxonomy_term']);

    $query = new EntityFieldQuery;
    $query->entityCondition('entity_type', 'node');
    $query->entityCondition('bundle', variable_get('os2intra_groups_node_type'));
    $query->fieldCondition(variable_get('os2intra_groups_organisation_connection_field'), variable_get('os2intra_groups_organisation_connection_reference'), $tid, '=');

    $result = $query->execute();

    if ($result) {
      return key($result['node']);
    }
  }
  return FALSE;
}
/*
 * Get department ID of default parent
 */

function _os2intra_user_import_get_default_department_id(){
  $vocab = taxonomy_vocabulary_load(variable_get('os2intra_groups_vocabulary_vid'));
  $tree = taxonomy_get_tree($vocab->vid);

  // find first term with no parents
  // used as default parent tid later
  foreach ($tree as $term) {
    if ($term->parents[0] == 0) {
      $default_parent = taxonomy_term_load($term->tid);
      return $default_parent->field_os2intra_department_id[LANGUAGE_NONE][0]['value'];
    }
  }
}
