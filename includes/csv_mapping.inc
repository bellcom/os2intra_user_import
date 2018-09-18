<?php
/**
 * Helper function to parse file contents to array with relevant info
 */
function _os2intra_user_import_process_file($file_path, $lines = FALSE) {
  $identification_method = variable_get('os2intra_user_import_activate_identification', OS2INTRA_USER_IMPORT_ACTIVATE_IDENTIFICATION);
  $rows = array();
  if (empty($identification_method)) {
    drupal_set_message('User activation mode is not setup. Please check user import configuration /admin/config/os2intra/user_import/settings', 'error');
    return $rows;
  }

  // CSV field mapping
  // rewrite for settings page?
  $mapping = variable_get('os2intra_mapping');
  $map_fallback = array(
    'dn' => 'os2intra_mapping_dn',
    'ad_id' => 'os2intra_mapping_ad_id',
    'ad_name' => 'os2intra_mapping_ad_name',
    'employee_id' => 'os2intra_mapping_employee_id',
    'first_name' => 'os2intra_mapping_first_name',
    'last_name' => 'os2intra_mapping_last_name',
    'department' => 'os2intra_mapping_department',
    'department_shortname' => 'os2intra_mapping_department_shortname',
    'centre' => 'os2intra_mapping_centre',
    'centre_name' => 'os2intra_mapping_centre_name',
    'job_description' => 'os2intra_mapping_job_description',
    'termination_date' => 'os2intra_mapping_termination_date',
    'start_date' => 'os2intra_mapping_start_date',
    'phone' => 'os2intra_mapping_phone',
    'mobile' => 'os2intra_mapping_mobile',
    'email' => 'os2intra_mapping_email',
    'birthdate' => 'os2intra_mapping_birthdate',
    'department_name' => 'os2intra_mapping_department_name',
    'opus_roles' => 'os2intra_mapping_opus_roles',
    'leader' => 'os2intra_mapping_leader',
    'has_ad' => 'os2intra_mapping_has_ad_value',
  );
  // Load data from file
  $csv = array_map(function ($d) {
    return str_getcsv($d, variable_get('os2intra_user_import_delimiter'));
  }, file($file_path));
  $at_content = FALSE;

  foreach ($csv as $row) {

    $mapped_row = array();
    foreach ($map_fallback as $field => $variable) {

      if (!empty($row[$mapping[$variable]['primary']])) {
        $mapped_row[$field] = _os2intra_user_import_sanitize_int($row[$mapping[$variable]['primary']]);
      }
      else {
        if ($mapping[$variable]['secondary'] == 'input') {
          $mapped_row[$field] = $mapping[$variable]['input'];
        }
        else {
          if (is_numeric($mapping[$variable]['secondary'])) {
            $mapped_row[$field] = _os2intra_user_import_sanitize_int($row[$mapping[$variable]['secondary']]);
          }
        }
      }
    }
    if (!isset($mapped_row["centre"])) {
      $mapped_row["centre"] = _os2intra_user_import_get_default_department_id();
    }
    if (!isset($mapped_row['centre_name'])) {
      $mapped_row['centre_name']= $mapped_row['centre'];
    }
    // Skip if row has no ad_id and no employee_id
    if ($mapped_row['ad_id'] == '' && $mapped_row['employee_id'] == '' ) {
      os2intra_user_import_save_log('', 'No ad_id and no employee_id found in row: ' . implode(';', $row));
      continue;
    }

    // Try to set ad_id from ad_name column.
    if (!isset($mapped_row['ad_id'])) {
      if (isset($mapped_row['ad_name']) && !empty($mapped_row['ad_name'])) {
        $mapped_row['ad_id'] = $mapped_row['ad_name'];
      }
    }

    $rows[] = $mapped_row;

    if ($lines && $lines < count($rows)) {
      break;
    }

  }

  // Skip first row, headers
  unset($rows[0]);
  return $rows;
}
