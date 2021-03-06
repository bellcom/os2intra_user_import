<?php
/**
 * Create department terms
 *
 * @param
 *  array of users with organisations
 */
function os2intra_user_import_groups($users) {
  // Array holding created terms. For email use later.
  $created_terms = array();

  os2intra_user_import_save_log('', 'Import organisation');

  $vocab = taxonomy_vocabulary_load(variable_get('os2intra_groups_vocabulary_vid'));
  $tree = taxonomy_get_tree($vocab->vid);

  // find first term with no parents
  // used as default parent tid later
  foreach ($tree as $term) {
    if ($term->parents[0] == 0) {
      $default_parent_tid = $term->tid;
      os2intra_user_import_save_log('', 'Default Parent ID: ' . $default_parent_tid);
      break;
    }
  }

  foreach ($users as $user) {
    if (empty($user['centre'])) {
      os2intra_user_import_save_log('', 'Skipping group. Parent department is missing. AD_ID:' . $user['ad_id'] . ' User ' . $user['first_name'] . ' ' . $user['last_name']);
      continue;
    } elseif (empty($user['department'])) {
      os2intra_user_import_save_log('', 'Skipping group. department is missing. AD_ID:' . $user['ad_id'] . ' User ' . $user['first_name'] . ' ' . $user['last_name']);
      continue;
    }

    $group = $user['department'];
    $parent_tid = $default_parent_tid;
    // Check if term already exists
    $result = os2intra_user_import_query_term_by_department($group);

    // Term already exists
    if (!empty($result)) {
      // Rename group term if needed
      $term = taxonomy_term_load(key($result['taxonomy_term']));
      $term->name = $user['department_name'];
      os2intra_user_import_save_log('', 'Looping over existing term: ' . $term->name);
      if ($term->tid != $parent_tid) {
        $term->parent = array($parent_tid);
      }
      if ($user['department'] != $user['centre']) {
        // Search for parent tid for department
        $result = os2intra_user_import_query_term_by_department($user['centre']);

        if (!empty($result)) {
          $parent_term = taxonomy_term_load(key($result['taxonomy_term']));
          $parent_tid = $parent_term->tid;

          if ($parent_term->tid != $term->tid) {
            $term->parent = array($parent_tid);
          }
        }
        else {
          $parent_term = new stdClass();
          $parent_term->name = $user['centre_name'];
          $parent_term->parent = array($parent_tid);
          $parent_term->field_os2intra_department_id[LANGUAGE_NONE][0]['value'] = $user['centre'];
          $parent_term->vid = $vocab->vid;
          if($tid = os2intra_user_import_get_parent_term_from_dn($user['dn'], $user['centre_name'], 'os2intra_organisation_tax')) {
            $parent_term->parent = array($tid);
          }
          taxonomy_term_save($parent_term);
          $parent_tid = $parent_term->tid;
          $term->parent = array($parent_tid);
          os2intra_user_import_create_department($parent_term->tid, $user['centre'], $user['centre_name'] );
        }
      }
      taxonomy_term_save($term);
      os2intra_user_import_create_department($term->tid, $user['department'], $user['department_name']);
    }
    else {
      // Create taxonomy terms
      os2intra_user_import_save_log('', 'Create department: ' . $group);


      if ($user['department'] != $user['centre']) {
        // Search for parent tid for department
        $result = os2intra_user_import_query_term_by_department($user['centre']);

        if (!empty($result)) {
          $parent_term = taxonomy_term_load(key($result['taxonomy_term']));
          $parent_tid = $parent_term->tid;
        }
        else {
          $parent_term = new stdClass();
          $parent_term->name = $user['centre_name'];
          $parent_term->parent = array($parent_tid);
          $parent_term->field_os2intra_department_id[LANGUAGE_NONE][0]['value'] = $user['centre'];
          $parent_term->vid = $vocab->vid;
          if (isset($user['dn'])) {
            if($tid = os2intra_user_import_get_parent_term_from_dn($user['dn'], $user['centre_name'], 'os2intra_organisation_tax')) {
              $parent_term->parent = array($tid);
            }
          }
          taxonomy_term_save($parent_term);
          $parent_tid = $parent_term->tid;
          os2intra_user_import_create_department($parent_term->tid, $user['centre'], $user['centre_name']);
        }
      }

      $term = new stdClass();
      $term->name = $user['department_name'];
      $term->parent = array($parent_tid);
      $term->field_os2intra_department_id[LANGUAGE_NONE][0]['value'] = $group;
      $term->field_os2intra_shortname[LANGUAGE_NONE][0]['value'] = $user['department_shortname'];
      $term->vid = $vocab->vid;

      taxonomy_term_save($term);
      $created_terms[] = $term;

      os2intra_user_import_create_department($term->tid, $user['department'], $user['department_name']);
    }
  }
  os2intra_user_import_save_log('', 'Finished importing groups');

  // Send email that new groups has been created
  if (count($created_terms) > 0) {
    _os2intra_user_import_send_email($created_terms);
  }
}

/**
 * Create department nodes
 */
function os2intra_user_import_create_department($tid, $department, $department_name) {

  // Check if department node already exists.
  $nid = _os2intra_user_import_get_department_nid($department);
  if ($nid) {
    // Rename group node name if needed
    $nid = _os2intra_user_import_get_department_nid($department);
    $node = node_load($nid);
    $node->title = $department_name;
    node_save($node);
    return;
  }

  // Find term title
  $term = taxonomy_term_load($tid);
  $name = $term->name;

  // Create group nodes
  $node = new stdClass();

  // Title and type
  $node->title = $name;
  $node->type = variable_get('os2intra_groups_node_type');

  // Generic node stuff
  node_object_prepare($node);

  $node->language = 'da';
  $node->uid = 1;
  $node->status = 1;

  // For some reason, we need to save the node and load it again, before we can assign values to fields
  $node = node_submit($node);
  node_save($node);

  // Set organization term
  $group_organisation_field = variable_get('os2intra_groups_organisation_connection_field');
  $group_organisation_reference = variable_get('os2intra_groups_organisation_connection_reference');
  $node->{$group_organisation_field}[LANGUAGE_NONE][][$group_organisation_reference] = $tid;

  // Save department id on node
  if (field_info_instance('node', 'field_os2intra_department_id', variable_get('os2intra_groups_node_type'))) {
    $node->field_os2intra_department_id[LANGUAGE_NONE][0]['value'] = $department;
  }

  // Group type "Organisation"
  if (field_info_instance('node', 'field_os2intra_grouptype', variable_get('os2intra_groups_node_type'))) {
    $os2intra_groups_organisation_tid = variable_get('os2intra_groups_organisation_tid');
    if (empty($os2intra_groups_organisation_tid)) {
      os2intra_user_import_save_log('', 'Group ' . $name . ' has incorrect settings. Organization group type definition is empty.');
    }
    $node->field_os2intra_grouptype[LANGUAGE_NONE][0]['tid'] = $os2intra_groups_organisation_tid;
  }

  // Closed group, require invitation
  if (field_info_instance('node', 'field_og_subscribe_settings', variable_get('os2intra_groups_node_type'))) {
    $node->field_og_subscribe_settings[LANGUAGE_NONE][0]['value'] = 'invitation';
  }

  $node = node_submit($node);
  node_save($node);
}

/**
 * Add user to parent departments
 */
function os2intra_user_import_add_user_to_parent_departments($account, $department_nid) {
  //$og_department_node = node_load(field_get_items('user', $account, variable_get('os2intra_user_organisation_field'))[0]['target_id']);
  $og_department_node = node_load($department_nid);
  if (empty($og_department_node)) {
    return;
  }
  $group_organisation_field = variable_get('os2intra_groups_organisation_connection_field');
  $group_organisation_reference = variable_get('os2intra_groups_organisation_connection_reference');
  $organisation_tid = field_get_items('node', $og_department_node, $group_organisation_field)[0][$group_organisation_reference];

  // Add user to their primary department and give them the 'medarbejder' role
  $values = array(
    'entity' => $account,
    'field_name' => variable_get('os2intra_user_primary_og_role_reference_field'),
  );
  og_group('node', $og_department_node->nid, $values);
  og_role_grant('node', $og_department_node->nid, $account->uid, variable_get('os2intra_user_import_og_role_id_primary', 16));
  // Add user to all parent departments and give 'organisatorisk' role on them.
  $parent_terms = taxonomy_get_parents_all($organisation_tid);
  if (isset($parent_terms)) {
    foreach ($parent_terms as $parent_term) {
      if ($parent_term->tid != $organisation_tid) {
        $parent_tids[] = $parent_term->tid;
      }
    }
    if (isset($parent_tids)) {
      // Find the node ID for each department in parent terms.
      $query = new EntityFieldQuery;
      $query->entityCondition('entity_type', 'node');
      $query->entityCondition('bundle', variable_get('os2intra_groups_node_type'));
      $query->fieldCondition($group_organisation_field, $group_organisation_reference, $parent_tids, 'IN');
      // Additional condition when there are few types of groups.
      if (field_info_instance('node', 'field_os2intra_grouptype', variable_get('os2intra_groups_node_type'))) {
        $query->fieldCondition('field_os2intra_grouptype', 'tid', variable_get('os2intra_groups_organisation_tid'));
      }

      // Added membership only for real department.
      if (field_info_instance('node', 'field_os2intra_department_id', variable_get('os2intra_groups_node_type'))) {
        $query->fieldCondition('field_os2intra_department_id', 'value', 'NULL', '!=');
      }
      $results = $query->execute();
      if (isset($results['node'])) {
        foreach ($results['node'] as $result) {
          $og_group_node_ids[] = $result->nid;
        }

        $values = array(
          'entity' => $account,
          'field_name' => variable_get('os2intra_user_parent_og_role_reference_field'),
        );
        foreach ($og_group_node_ids as $nid) {
          // Add the user to the group and set role.
          og_group('node', $nid, $values);
          if (os2intra_user_import_is_user_department_manager($account->uid, $nid)) {
            og_role_grant('node', $nid, $account->uid, variable_get('os2intra_user_import_og_role_id_manager', 15));
          }
          else {
            og_role_grant('node', $nid, $account->uid, variable_get('os2intra_user_import_og_role_id_parent', 14));
          }
         // og_role_grant('node', $nid, $account->uid, variable_get('os2intra_user_import_og_role_id_parent', 14));
        }
      }
    }
  }
}

function os2intra_user_import_get_parent_term_from_dn($dn_string, $term_name, $vid) {
preg_match_all('/OU=[^,]+/', $dn_string, $matches);
$matches =  str_replace ('OU=', '', reset($matches));
foreach ($matches as $key => $value) {
if ($value == $term_name) {
  $parent_term_name  = $matches[$key+1];
 break;
 }
}
$parent_term = reset(taxonomy_get_term_by_name($parent_term_name));
if ($parent_term) {
  return $parent_term->tid;
}
else
  return FALSE;
}
