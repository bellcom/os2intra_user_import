<?php
/**
 * Disable users according to termination date
 */
function os2intra_user_import_disable_users() {
  os2intra_user_import_save_log('', 'Start disabling users');
  $timestamp = strtotime('midnight tomorrow');
  if (variable_get('os2intra_user_import_user_disable_action', 'user_disable') == 'user_delete_reassign') {
    $query = new EntityFieldQuery;
    $query->entityCondition('entity_type', 'user');
    $query->fieldCondition('field_os2intra_termination_date', 'value', $timestamp, '<');
    $query->fieldCondition('field_os2intra_termination_date', 'value', 1, '>');
    $result = reset($query->execute());

    $method = 'user_cancel_reassign';
    // Delete  users
    if (is_array($result)) {
      foreach ($result as $uid => $item) {
        $account =  user_load($uid);
        $user_name = $account->field_name_first['und'][0]['value'] . ' ' . $account->field_name_last['und'][0]['value'] . ' (' . $account->mail . ')';
        module_invoke_all('user_cancel', array(), $account, $method);
        user_delete($uid);
        os2intra_user_import_save_log('', 'Delete user: ' . $user_name);
      }
    }

  }
  else {
    // Query all users with termination date today.
    $query = new EntityFieldQuery;
    $query->entityCondition('entity_type', 'user');
    $query->fieldCondition('field_os2intra_termination_date', 'value', $timestamp, '<');
    $query->fieldCondition('field_os2intra_termination_date', 'value', 1, '>');
    $query->propertyCondition('status', 1);
    $result = reset($query->execute());

    // Array holding new status for user
    $disable_array = array('status' => 0);

    // Disable users
    if (is_array($result)) {
      foreach ($result as $uid => $item) {
        os2intra_user_import_save_log('', 'Disable uid: ' . $uid);
        user_save(user_load($uid), $disable_array);
      }
    }
  }
  os2intra_user_import_save_log('', 'User disabling finished');
}

/**
 * Check if the user is already created in the system
 */
function os2intra_user_import_check_users(&$users, $duplicate_emails, $duplicate_ids) {
  $update_users = array();
  foreach ($users as $key => $user) {
    if (array_search($user['email'], $duplicate_emails) !== false) {
      unset($users[$key]);
      continue;
    }

    if (array_search($user['ad_id'], $duplicate_ids) !== false) {
      unset($users[$key]);
      continue;
    }

    $result = NULL;

    $identification_method = variable_get('os2intra_user_import_activate_identification');
    if ($identification_method == 'ad_id') {
      $query = new EntityFieldQuery;
      $query->entityCondition('entity_type', 'user');
      $query->propertyCondition('name', $user['ad_id']);
      $result = $query->execute();
    } elseif ($identification_method == 'employee_id') {
      $query = new EntityFieldQuery;
      $query->entityCondition('entity_type', 'user');
      $query->fieldCondition('field_os2intra_employee_id', 'value', $user['employee_id']);
      $result = $query->execute();
    }

    // Unset users already created
    if ($result) {
      $update_users[key(array_shift($result))] = $user;
      unset($users[$key]);
    }
  }

  return $update_users;
}

/**
 * Create/update user
 * If a uid is sent the user will be updated, otherwise a new user will be created.
 * The user has multiple group connections, some are from the import file, and some
 * are set manually in the system. In order to preserve the manually set groups we
 * save the imported values in field_os2intra_import_groups, we then diff this
 * field with og_user_node to find out what groups are set manually (the diff).
 * These are then readded to the users groups, along with the organisational groups
 * from the import file.
 * Group id 2 (Odsherred kommune) is a different story, all users should be
 * members of this group, and therefore we add it manually to all.
 *
 * @author Thomas Thune Hansen <tth@bellcom.dk>
 *
 * @param
 *  user array from file parser
 *
 */
function os2intra_user_import_save_user($user, $uid = '') {
  // Load users account, if any
  $account = user_load($uid);

  $keep_groups = array();
  $no_import_groups = FALSE;

  // Load users values from account
  if (is_object($account)) {
    $og_user_node = array();
    $import_groups = array();
    // Get a list of current group nids
    if ($og_user_node_field = field_get_items('user', $account, variable_get('os2intra_user_group_field'))) { // gruppemedlemskab
      foreach ($og_user_node_field as $key => $field) {
        $og_user_node[$field['target_id']] = $field['target_id'];
      }
    }
    // And organisation and centre nid from last import
    if ($import_group_field = field_get_items('user', $account, variable_get('os2intra_user_department_field'))) { // afdeling
      foreach ($import_group_field as $key => $field) {
        $import_groups[$field['target_id']] = $field['target_id'];
      }
    }
    // The diff will be the groups the user should keep.
    if (!empty($import_groups)) {
      $keep_groups = array_diff($og_user_node, $import_groups);
      // Group id 2 is unset as we add this to all users later.
      unset($keep_groups[2]);
    }
    else {
      // Keep all groups, if there are no import groups
      $keep_groups = $og_user_node;
      $no_import_groups = TRUE;
    }
  }

  // Get node id for group
  $query = new EntityFieldQuery;
  $query->entityCondition('entity_type', 'node');

  $group_department_title_field = variable_get('os2intra_user_import_group_department_title_field');
  // Property field
  if (strpos($group_department_title_field, 'field_') !== 0) {
    $query->propertyCondition($group_department_title_field, $user['department']);
  } else {
    $query->fieldCondition($group_department_title_field, 'value', $user['department'], '=');
  }

  $result = $query->execute();

  if (!empty($result['node'])) {

    $new_group_nid = key(array_shift($result));

    // Attach new group.
    // We rebuild the association completely so we start with an empty
    // array.
    $user_groups = array();
    if (variable_get('os2intra_user_default_groups')) {
      $def_groups = explode(',', variable_get('os2intra_user_default_groups'));
      foreach ($def_groups as $def_group) {
        $user_groups[LANGUAGE_NONE][]['target_id'] = $def_group;
      }
    }
    $user_groups[LANGUAGE_NONE][]['target_id'] = $new_group_nid;

    // In field_os2intra_import_groups we store the group ids
    // imported, for both organisation unit and center.
    // On the next import, we need this to find out what groups the
    // user has been added to, that are not organisational.
    $import_groups = array();
    $import_groups[LANGUAGE_NONE][]['target_id'] = $new_group_nid;
  }

  $og_user_node = array();
  // Get a list of current nids
  if ( $og_user_node_field = field_get_items('user', $account, 'og_user_node') ) {
    foreach ($og_user_node_field as $key => $field) {
      if (isset($field['target_id']) && node_load($field['target_id']) )    {
        $og_user_node[]['target_id'] = $field['target_id'];
      }
    }
  }

  // Get node id for centre
  $query = new EntityFieldQuery;
  $query->entityCondition('entity_type', 'taxonomy_term');
  $query->fieldCondition('field_os2intra_department_id', 'value', $user['centre'], '=');
  $result = $query->execute();


  if (!empty($result['taxonomy_term'])) {
    $centre_nid = key(array_shift($result));
    $user_groups[LANGUAGE_NONE][]['target_id'] = $centre_nid;

    // Don't do duplicates
    if ($new_group_nid !== $centre_nid) {
      $import_groups[LANGUAGE_NONE][]['target_id'] = $centre_nid;
    }
  }

  // If there are no import groups, we add the groups, the user had at import.
  // import groups will be set during this import, so next time, this should
  // not be an issue.
  // This is only done in order to migrate user groups, from users before the
  // import groups field was added.
  if ($no_import_groups) {
    $user_groups = array();
  }

  // Add back the groups we want to keep.
  foreach ($keep_groups as $group_id) {
    // Only the existing nodes
    if (node_load($group_id) !== FALSE ) {
      $user_groups[LANGUAGE_NONE][]['target_id'] = $group_id;
    }
  }

  // @todo: check that timestamp is not today
  if (array_key_exists('termination_date', $user)) {
    // Check if termination date is formatted as date
    $termination_date = date_create_from_format(variable_get('os2intra_user_import_date_format', 'd.m.y H:i:s') , $user['termination_date'] . ' 00:00:00');
    if ($termination_date) {
      $termination_date_timestamp = date_timestamp_get($termination_date);
    } else {
      $termination_date_timestamp = $user['termination_date'];
    }

    // Check if the timestamp supplied from the import is larger than
    // what unix timestamps can be.
    if ($termination_date_timestamp > 2147483647 or $termination_date_timestamp == FALSE ) {
      $termination_date_timestamp = 1;
    }
  }

  // Create Opus Roles and set them on user
  $keep_opus_roles = array();
  $opus_role_tids = array();
  $opus_role_vocabulary = taxonomy_vocabulary_machine_name_load('os2intra_bi_opus_roles');
  $opus_roles = array();

  if ($uid) {
    foreach ($account->os2intra_users_opus_roles as $os2intra_users_opus_role) {
      foreach ($os2intra_users_opus_role as $key => $value) {
        $keep_opus_roles[] = $value;
      }
    }
  }
  $opus_roles =  explode(',', $user['opus_roles']);
  os2intra_user_import_create_opus_roles($opus_roles);

  foreach ($opus_roles as $opus_role) {
    $query = new EntityFieldQuery;
    $result = $query
      ->entityCondition('entity_type', 'taxonomy_term')
      ->propertyCondition('name', trim($opus_role))
      ->propertyCondition('vid', $opus_role_vocabulary->vid)
      ->execute();

    if (isset($result['taxonomy_term'])) {
      foreach ($result['taxonomy_term'] as $term) {
        $opus_role_tids[] = $term->tid;
      }
    }
  }
  $opus_role_tids = array_diff($opus_role_tids, $keep_opus_roles);

  // User title tid
  $title_tid = os2intra_user_import_user_title($user['job_description']);

  // Birthday
  // For some reason, the first zero is chopped somewhere. Let's add it.
  if (array_key_exists('birthdate', $user) && strlen($user['birthdate']) < 6) {
    $user['birthdate'] = '0' . $user['birthdate'];
  }
  $user['birthdate'] = date_format(date_create_from_format(variable_get('os2intra_user_import_date_format', 'd.m.y H:i:s'), $user['birthdate'] . ' 00:00:00'), variable_get('os2intra_user_import_birthdate_db_format'));

  // Get user membership to department
  $department_nid = _os2intra_user_import_get_department_nid($user['department']);

  // Populate fields
  $fields = array(

    // Set employee_id
    'field_os2intra_employee_id'      => array(LANGUAGE_NONE => array(0 => array('value' => $user['employee_id']))),

    // AD name
    'field_os2intra_ad_name'          => array(LANGUAGE_NONE => array(0 => array('value' => $user['ad_name']))),

    // First Name
    'field_name_first'                => array(LANGUAGE_NONE => array(0 => array('value' => $user['first_name']))),

    // Last Name
    'field_name_last'                 => array(LANGUAGE_NONE => array(0 => array('value' => $user['last_name']))),

    // Phone
    'field_os2intra_phone'            => array(LANGUAGE_NONE => array(0 => array('value' => (array_key_exists('phone', $user)) ? $user['phone'] : ''))),

    // Mobile
    'field_os2intra_mobile'            => array(LANGUAGE_NONE => array(0 => array('value' => (array_key_exists('mobile', $user)) ? $user['mobile'] : ''))),

    // Set birthdate
    variable_get('os2intra_user_import_birthday_field') => array(LANGUAGE_NONE => array(0 => array('value' => (array_key_exists('birthdate', $user)) ? $user['birthdate'] : NULL))),

    //Set users nodes
    variable_get('os2intra_user_group_field') => array(LANGUAGE_NONE => $og_user_node),

    // Set department membership
    variable_get('os2intra_user_department_field') => array(LANGUAGE_NONE => array(0 => array('target_id' => array($department_nid)))),

    // Activate user
    'status'                          => ($account->uid != 0) ? $account->status : 1,

    'timezone'                        => 'Europe/Copenhagen',
    // The organisational ids from this import
    'field_os2intra_import_groups'    => $import_groups,
  );

  if (isset($user['email'])) {
    $fields['mail'] = $user['email'];
    $fields['init'] = $user['email'];
  }
  if (isset($user['phone'])) {
    $fields['field_os2intra_phone'] = array(LANGUAGE_NONE => array(0 => array('value' => $user['phone'])));
  }
  if (isset($user['mobile'])) {
    $fields['field_os2intra_mobile'] = array(LANGUAGE_NONE => array(0 => array('value' => $user['mobile'])));
  }

  // Job description
  if ($title_tid) {
    $fields['field_os2intra_user_titles'] = array(LANGUAGE_NONE => array(0 => array(variable_get('os2intra_user_import_job_title_connection_reference') => $title_tid)));
  }

  // Termination date
  if (isset($termination_date_timestamp)) {
    $fields['field_os2intra_termination_date'] = array(LANGUAGE_NONE => array(0 => array('value' => $termination_date_timestamp)));
  }

  // Handle whether we're updating or creating a new user
  // if we're updating we don't generate username and sets password
  if (!is_numeric($uid)) {
    $password = user_password(8);

    // At this step even if the ad_id was empty initially it must have been generated
    $fields['name'] = $user['ad_id'];
    $fields['pass'] = $password;
  }


  // Set Opus Roles
  if (field_info_instance('user', 'os2intra_users_opus_roles', 'user')) {
    $fields['os2intra_users_opus_roles'][LANGUAGE_NONE] = array();
    foreach ($opus_role_tids as $opus_role_tid) {
      $fields['os2intra_users_opus_roles'][LANGUAGE_NONE][]['tid'] = $opus_role_tid;
    }
  }

  // Save user
  $account = user_save($account, $fields);

  // Remove all department memberships
  $gid_types = og_get_groups_by_user($account);
  if (is_array($gid_types)) {
    foreach ($gid_types as $gid_type) {
      foreach ($gid_type as $gid) {
        og_role_revoke('node', $gid, $account->uid, variable_get('os2intra_user_import_og_role_id_primary', 16));
      }
    }
  }

  // Set department membership and role
  $values = array(
    'entity' => $account,
    'field_name' => variable_get('os2intra_user_group_field'),
  );
  og_group('node', $department_nid, $values);
  og_role_grant('node', $department_nid, $account->uid, variable_get('os2intra_user_import_og_role_id_primary', 16));

  //create subscription to user's department
  $user_subscriptions = flag_get_user_flags('node', $department_nid, $account->uid);
  if (!isset($user_subscriptions['subscribe_og'])) {
    flag('flag', 'subscribe_og', $department_nid, $account, true);
  }

  if (!empty($user['ad_name'])) {
    // Create authmap for user
    $authmap = array(
      'authname_simplesamlphp_auth' => $user['ad_name'] . variable_get('os2intra_user_import_authmap_name_suffix'),
    );
  } elseif (!empty($user['ad_id'])) {
    // Create authmap for user
    $authmap = array(
      'authname_simplesamlphp_auth' => $user['ad_id'] . variable_get('os2intra_user_import_authmap_name_suffix'),
    );
  }

  if (!user_get_authmaps($authmap)) {
    user_set_authmaps($account, $authmap);
  }

  // Add users to parent departments
  if (variable_get('os2intra_add_users_to_parent_departments')) {
    os2intra_user_import_add_user_to_parent_departments($account);
  }

  // Write log entry
  if ($uid) {
    os2intra_user_import_save_log($user['employee_id'], 'update user: ' . $user['employee_id'] . ' ' . $user['first_name'] . ' ' . $user['last_name'] . 'status: ' . $account->uid);
  }
  else {
    os2intra_user_import_save_log($user['employee_id'], 'create user: ' . $user['first_name'] . 'status: ' . $account->uid);
  }
}

/*
 * helper function for generate username
 */
function os2intra_user_import_generate_username($user) {
  // Generate username
  // Todo: make configurable
  $cln_first_name = preg_replace('/[^A-Za-z0-9\-]/', '', $user['first_name']);
  $cln_last_name = preg_replace('/[^A-Za-z0-9\-]/', '', $user['last_name']);
  $cln_department = preg_replace('/[^A-Za-z0-9\-]/', '', $user['department']);

  $username = strtolower(substr($cln_department, 0, 3)) . '-';
  $username .= strtolower(substr($cln_first_name, 0, 3));
  $username .= strtolower(substr($cln_last_name, 0, 3));
  
//FIXME: add employee_id to username
  //$username .= $user['employee_id'];

  // FIXME: not allowing name increase
//  $users_query = db_query('SELECT u.name FROM {users} u WHERE u.name LIKE :name', array(':name' => $username.'%'));
//  $count = count($users_query->fetchAll());
//
//  if($count){
//    $username .= $count + 1;
//  }
  return $username;
}