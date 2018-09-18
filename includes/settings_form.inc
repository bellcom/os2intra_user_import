<?php
/**
 * Form callback for settings
 */
function os2intra_user_import_settings() {
  drupal_add_css(drupal_get_path('module', 'os2intra_user_import') . '/css/os2intra_user_import.css');
  drupal_add_js(drupal_get_path('module', 'os2intra_user_import') . '/js/os2intra_user_import.js');
 $import_dir = variable_get('os2intra_user_import_dir', 'private://user_import');

    $realpath = '';

    $filename = variable_get('os2intra_user_import_file_name', 'FK-user_import');
    $files = file_scan_directory($import_dir, '/' . $filename . '.csv/');
    $dir = reset($files);
    if ($dir) {
      $realpath = drupal_realpath($dir->uri);
    }

    if (file_exists($realpath)) {
        $users_array = _os2intra_user_import_process_file($realpath, 1);
        $users =  reset($users_array);
     }
    else{
      drupal_set_message('Csv file is not exists','warning');
    }
   $csv_cols = array(
    'none' => t('None'),
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
  );
$csv_cols_fallback_1 = array(
    'none' => t('None'),
    'input' => t('Input'),
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
  );
  $form['placeholder'] = array(
    '#type'   => 'item',
    '#markup' => t('This module does not provide a cron hook. This has to be set up manually, by adding the import script to the crontab'),
  );

  $form['os2intra_user_import_enable'] = array(
    '#title'         => t('Enable user import'),
    '#type'          => 'checkbox',
    '#default_value' => variable_get('os2intra_user_import_enable', FALSE)
  );

  $form['os2intra_user_import_dir'] = array(
    '#title'         => t('Directory containing CSV with users'),
    '#type'          => 'textfield',
    '#required'      => TRUE,
    '#description'   => t('ex. private://user_import'),
    '#default_value' => variable_get('os2intra_user_import_dir', 'private://user_import')
  );
  $form['os2intra_user_import_file_name'] = array(
    '#title'         => t('CSV file name'),
    '#type'          => 'textfield',
    '#required'      => TRUE,
    '#description'   => t('ex. FK-user_import (without extension .csv, without space)'),
    '#default_value' => variable_get('os2intra_user_import_file_name', 'FK-user_import'),
  );
  $form['os2intra_user_import_delimiter'] = array(
    '#title'         => t('CSV delimiter'),
    '#type'          => 'textfield',
    '#description'   => t('The delimiter used to separate values in the CSV, ex. a semicolon, a comma or \t (tab)'),
    '#default_value' => variable_get('os2intra_user_import_delimiter', ';'),
  );
  $form['os2intra_user_import_date_format'] = array(
    '#title'         => t('Date format'),
    '#type'          => 'textfield',
    '#description'   => t('Date format in CSV file. See possible values in the <a href="http://php.net/manual/en/function.date.php" target="_blank">PHP manual</a>.'),
    '#default_value' => variable_get('os2intra_user_import_date_format', 'd.m.y H:i:s'),
  );
  $form['os2intra_user_default_groups'] = array(
    '#title'         => t('Node ids (nid) of default OG groups, that should be added to every imported used'),
    '#type'          => 'textfield',
    '#description'   => t('CSV, ex. 1,2,3'),
    '#default_value' => variable_get('os2intra_user_default_groups')
  );

  $form['os2intra_user_notification_email'] = array(
    '#title'         => t('Email address that should be notified when new departments are created'),
    '#type'          => 'textfield',
    '#default_value' => variable_get('os2intra_user_notification_email', variable_get('site_mail')),
  );

  $form['os2intra_user_import_og_role_id_parent'] = array(
    '#title'         => t('Parent OG Role Grant ID'),
    '#type'          => 'textfield',
    '#default_value' => variable_get('os2intra_user_import_og_role_id_parent', 14),
    '#description'   => t('ID of the group role to grant the user, when added to parent groups.'),
  );

  $form['os2intra_user_import_og_role_id_primary'] = array(
    '#title'         => t('Primary OG Role Grant ID'),
    '#type'          => 'textfield',
    '#default_value' => variable_get('os2intra_user_import_og_role_id_primary', 16),
    '#description'   => t('ID of the group role to grant the user, when added to primary group.'),
  );

  $form['os2intra_user_import_og_role_id_manager'] = array(
    '#title'         => t('Manager OG Role Grant ID'),
    '#type'          => 'textfield',
    '#default_value' => variable_get('os2intra_user_import_og_role_id_manager', 15),
    '#description'   => t('ID of the group role to grant the user, when added to manager group.'),
  );

  $form['os2intra_user_import_authmap_name_suffix'] = array(
    '#title'         => t("Suffix used on the column 'authname' in the authmap table"),
    '#type'          => 'textfield',
    '#default_value' => variable_get('os2intra_user_import_authmap_name_suffix'),
    '#description'   => t("For example '@fredericia.dk'"),
  );
  $form['os2intra_user_import_user_disable_action'] = array(
    '#title'         => t("Action that sould be done when user disabled"),
    '#type'          => 'radios',
    '#options' =>  array('user_disable' => t('Disable user'), 'user_delete_reassign' => t('Delete the account and assign its contents to the anonymous user')),
    '#default_value' => variable_get('os2intra_user_import_user_disable_action', 'user_disable'),
  );

  // User activation settings, see includes/activation_form.inc
  $form['os2intra_user_import_activate'] = array(
    '#type' => 'fieldset',
    '#title' => t('User activation'),
    '#collapsible' => TRUE,
    '#weight' => -100,
  );

  $form['os2intra_user_import_activate']['activation_fields'] = array(
    '#type' => 'fieldset',
    '#title' => t('Validate first login by'),
  );

  $form['os2intra_user_import_activate']['activation_fields']['os2intra_user_import_activate_identification'] = array(
    '#title' => t('User identification'),
    '#type' => 'select',
    '#required' => TRUE,
    '#options' => array(
      '' => t('None'),
      'ad_id' => t('AD id'),
      'employee_id' => ('Employee number'),
    ),
    '#default_value' => variable_get('os2intra_user_import_activate_identification', OS2INTRA_USER_IMPORT_ACTIVATE_IDENTIFICATION),
  );

  $form['os2intra_user_import_activate']['activation_fields']['os2intra_user_import_activate_birthday'] = array(
    '#title' => t('Birthday'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('os2intra_user_import_activate_birthday', OS2INTRA_USER_IMPORT_ACTIVATE_BIRTHDAY),
  );

  $form['os2intra_user_import_activate']['activation_fields']['os2intra_user_import_activate_email'] = array(
    '#title' => t('Email address'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('os2intra_user_import_activate_email', OS2INTRA_USER_IMPORT_ACTIVATE_EMAIL),
  );

  // Settings for data structure compatibility in different installations.
  $form['os2intra_structure_settings'] = array(
    '#type'          => 'fieldset',
    '#title'         => t('Structure settings'),
    '#collapsible'   => TRUE,
  );


  $available_vocabularies = array();
  $vocabularies_list = taxonomy_get_vocabularies();
  foreach($vocabularies_list as $vocabulary) {
    $available_vocabularies[$vocabulary->vid] = $vocabulary->name;
  }
  asort($available_vocabularies);
  // Variable which contains organization vocabulary ID.
  $form['os2intra_structure_settings']['os2intra_groups_vocabulary_vid'] = array(
    '#type'          => 'select',
    '#title'         => t('Groups vocabulary name'),
    '#description'   => t('Taxonomy vocabulary which contains organisation terms'),
    '#required'      => TRUE,
    '#options'       => $available_vocabularies,
    '#default_value' => variable_get('os2intra_groups_vocabulary_vid')
  );
  // Variable which contains user's job titles vocabulary ID.
  $form['os2intra_structure_settings']['os2intra_job_title_vocabulary_vid'] = array(
    '#type'          => 'select',
    '#title'         => t('Job title vocabulary name'),
    '#description'   => t('Taxonomy vocabulary which contains job titles'),
    '#required'      => TRUE,
    '#options'       => $available_vocabularies,
    '#default_value' => variable_get('os2intra_job_title_vocabulary_vid')
  );

  $available_node_types = array();
  $node_types_list = node_type_get_types();
  foreach($node_types_list as $node_type) {
    $available_node_types[$node_type->type] = $node_type->name;
  }
  asort($available_node_types);
  // Variable which contains  content type what used for groups.
  $form['os2intra_structure_settings']['os2intra_groups_node_type'] = array(
    '#type'          => 'select',
    '#title'         => t('Groups node type'),
    '#description'   => t('Content type which used for organizations'),
    '#required'      => TRUE,
    '#options'       => $available_node_types,
    '#default_value' => variable_get('os2intra_groups_node_type')
  );

  if (variable_get('os2intra_groups_node_type')) {
    $group_type_fields = field_info_instances('node', variable_get('os2intra_groups_node_type'));
    $group_type_fields_select = array('title' => 'title');
    foreach ($group_type_fields as $field_name => $value) {
      $group_type_fields_select[$field_name] = $field_name;
    }
    // Variable which contains content type what used for groups.
    $form['os2intra_structure_settings']['os2intra_groups_organisation_connection_field'] = array(
      '#type'          => 'select',
      '#title'         => t('Groups -> organisation connection field'),
      '#description'   => t('Field name in organization group node which is used for connecting with organization taxonomy'),
      '#required'      => TRUE,
      '#options'       => $group_type_fields_select,
      '#default_value' => variable_get('os2intra_groups_organisation_connection_field')
    );
    // Variable which contains type of reference which used for connection between group node and organization taxonomy.
    $form['os2intra_structure_settings']['os2intra_groups_organisation_connection_reference'] = array(
      '#type'          => 'select',
      '#title'         => t('Groups -> organisation connection reference type'),
      '#description'   => t('Reference type to organization taxonomy in organization group node (Term Reference or Entity Reference)'),
      '#required'      => TRUE,
      '#options'       => array('target_id' => 'target_id', 'tid' => 'tid'),
      '#default_value' => variable_get('os2intra_groups_organisation_connection_reference')
    );
    // Variable which contains the name of field  in group node which used as group title.
    $form['os2intra_structure_settings']['os2intra_user_import_group_department_title_field'] = array(
      '#type'          => 'select',
      '#title'         => t('Groups department title field'),
      '#description'   => t('The name of field in organization node which is used as organization title'),
      '#required'      => TRUE,
      '#options'       => $group_type_fields_select,
      '#default_value' => variable_get('os2intra_user_import_group_department_title_field')
    );

    if (isset($group_type_fields['field_os2intra_grouptype'])) {
      // For case when group has group type field we have to define
      // Organizational term.
      $vid = taxonomy_vocabulary_machine_name_load('os2intra_tax_group_type')->vid;
      $options = array('' => 'None');
      foreach (taxonomy_get_tree($vid) as $item) {
        $key = $item->tid;
        $value = $item->name;
        $options[$key] = $value;
      }
      $form['os2intra_structure_settings']['os2intra_groups_organisation_tid'] = array(
        '#type'          => 'select',
        '#title'         => t('Organization term'),
        '#description'   => t('Gruppe Type taxonomy term define groups as Organization'),
        '#required'      => TRUE,
        '#options'       => $options,
        '#default_value' => variable_get('os2intra_groups_organisation_tid'),
      );
    }

  } else {
    $form['os2intra_structure_settings']['os2intra_groups_organisation_connection_field'] = array(
      '#markup' => t('Please select "Groups node type" and save the form before you can select "Groups -> organisation connection field"')
    );
  }

  $user_type_fields = field_info_instances('user', 'user');
  $user_type_fields_select = array();
  foreach ($user_type_fields as $field_name => $value) {
    $user_type_fields_select[$field_name] = $field_name;
  }
  // Variable which contains the name of user profile field  which used for save references to user's groups.
  $form['os2intra_structure_settings']['os2intra_user_group_field'] = array(
    '#type'          => 'select',
    '#title'         => t('User group reference field'),
    '#description'   => t('The name of field in the user profile which contains references to user\'s groups nodes'),
    '#required'      => TRUE,
    '#options'       => $user_type_fields_select,
    '#default_value' => variable_get('os2intra_user_group_field')
  );
  // Variable which contains the name of user profile field  which used for save references to all user's organizations(departments).
  $form['os2intra_structure_settings']['os2intra_user_organisation_field'] = array(
    '#type'          => 'select',
    '#title'         => t('User organisation reference field'),
    '#description'   => t('The name of field in the user profile which contains references to user\'s organisations nodes'),
    '#required'      => TRUE,
    '#options'       => array_merge(array('none' => t('None')), $user_type_fields_select),
    '#default_value' => variable_get('os2intra_user_organisation_field')
  );

  // Variable which contains the name of user profile field  which used for save reference to own user's organization(department).
  $form['os2intra_structure_settings']['os2intra_user_department_field'] = array(
    '#type'          => 'select',
    '#title'         => t('User department reference field'),
    '#description'   => t('The name of field in user profile which contains reference to user\'s own department'),
    '#required'      => TRUE,
    '#options'       => $user_type_fields_select,
    '#default_value' => variable_get('os2intra_user_department_field')
  );
  // Variable which contains the name of user profile field  the membership  with "Primary OG Role" should be registered in.
  $form['os2intra_structure_settings']['os2intra_user_primary_og_role_reference_field'] = array(
    '#type'          => 'select',
    '#title'         => t('Primary OG Role reference_field'),
    '#description'   => t('The name of the field, the membership  with "Primary OG Role" should be registered in'),
    '#required'      => TRUE,
    '#options'       => $user_type_fields_select,
    '#default_value' => variable_get('os2intra_user_primary_og_role_reference_field')
  );
  // Variable which contains the name of user profile field the membership  with "Parent OG Role" should be registered in.
  $form['os2intra_structure_settings']['os2intra_user_parent_og_role_reference_field'] = array(
    '#type'          => 'select',
    '#title'         => t('Parent OG Role reference field'),
    '#description'   => t('The name of the field, the membership  with "Parent OG Role" should be registered in'),
    '#required'      => TRUE,
    '#options'       => $user_type_fields_select,
    '#default_value' => variable_get('os2intra_user_parent_og_role_reference_field')
  );
  // Variable which contains the name of user profile field which contains user\'s birthday.
  $form['os2intra_structure_settings']['os2intra_user_import_birthday_field'] = array(
    '#type'          => 'select',
    '#title'         => t('User birthday field'),
    '#description'   => t('Field in user profile which contains user\'s birthday'),
    '#required'      => TRUE,
    '#options'       => $user_type_fields_select,
    '#default_value' => variable_get('os2intra_user_import_birthday_field')
  );
  // Variable which contains the name of user profile field which contains user\'s job title fields.
  $form['os2intra_structure_settings']['os2intra_user_import_job_title_field'] = array(
    '#type'          => 'select',
    '#title'         => t('User\'s job title field'),
    '#description'   => t('Field in user profile which contains user\'s job title'),
    '#required'      => TRUE,
    '#options'       => $user_type_fields_select,
    '#default_value' => variable_get('os2intra_user_import_job_title_field')
  );
  // Variable which contains the name of user profile field which contains user\'s firstname.
  $form['os2intra_structure_settings']['os2intra_user_import_firstname_field'] = array(
    '#type'          => 'select',
    '#title'         => t('User\'s firstname'),
    '#description'   => t('Field in user profile which contains user\'s firstname'),
    '#required'      => TRUE,
    '#options'       => $user_type_fields_select,
    '#default_value' => variable_get('os2intra_user_import_firstname_field')
  );
  // Variable which contains the name of user profile field which contains user\'s lasttname.
  $form['os2intra_structure_settings']['os2intra_user_import_lastname_field'] = array(
    '#type'          => 'select',
    '#title'         => t('User\'s firstname'),
    '#description'   => t('Field in user profile which contains user\'s lastname'),
    '#required'      => TRUE,
    '#options'       => $user_type_fields_select,
    '#default_value' => variable_get('os2intra_user_import_lastname_field')
  );
  // Variable which contains settings to determinate if user should be added  as members to parent departmnets.
  $form['os2intra_structure_settings']['os2intra_add_users_to_parent_departments'] = array(
    '#type'          => 'checkbox',
    '#description'   => t('If checked, users will be added as member into parent departments with "Parent OG Role"'),
    '#title'         => t('Add users to parent departments'),
    '#default_value' => variable_get('os2intra_add_users_to_parent_departments')
  );
  // Variable which contains settings to determinate if user should be added  as manager to parent departmnets.
  $form['os2intra_structure_settings']['os2intra_add_managers_to_departments'] = array(
    '#type'          => 'checkbox',
    '#description'   => t('If checked, user will be added as manager to parents departments'),
    '#title'         => t('Add managers to departments'),
    '#default_value' => variable_get('os2intra_add_managers_to_departments')
  );
  // Variable which contains settings to determinate if old references to user's departments should saved during import.
  $form['os2intra_structure_settings']['os2intra_save_old_departments'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Save old departments references during import'),
    '#description'   => t('If unchecked only current department reference will be saved for user'),
    '#default_value' => variable_get('os2intra_save_old_departments')
  );
  // Variable which contains settings to determinate if old user memberships should saved during import.
  $form['os2intra_structure_settings']['os2intra_revoke_og_roles'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Save old departments memberships'),
    '#description'   => t('If checked only membership from current import will be added'),
    '#default_value' => variable_get('os2intra_revoke_og_roles')
  );
  // Variable which contains birthday format.
  $form['os2intra_structure_settings']['os2intra_user_import_birthdate_db_format'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Birthdate expected format (to write into database)'),
    '#default_value' => variable_get('os2intra_user_import_birthdate_db_format'),
    '#description'   => t('Date format. See possible values in the <a href="http://php.net/manual/en/function.date.php" target="_blank">PHP manual</a>.')
  );
  // Variable which contains type of reference which used for connection between user profile and job title taxonomy.
  $form['os2intra_structure_settings']['os2intra_user_import_job_title_connection_reference'] = array(
    '#type'          => 'select',
    '#title'         => t('Users -> job title connection reference type'),
    '#description'   => t('Reference type to job taxonomy in user profie(Term Reference or Entity Reference)'),
    '#required'      => TRUE,
    '#options'       => array('target_id' => 'target_id', 'tid' => 'tid'),
    '#default_value' => variable_get('os2intra_user_import_job_title_connection_reference')
  );
  // Mapping settings
  $form['os2intra_mapping'] = array(
    '#type'          => 'fieldset',
    '#title'         => t('Mapping'),
    '#collapsible'   => TRUE,
    '#tree'          => TRUE,
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fieldset')
    )
  );
  $form['os2intra_mapping']['description'] = array(
    '#type'   => 'item',
    '#markup' => t('Select columns in the CSV file corresponding to the data'),
  );
  $form['os2intra_mapping']['os2intra_mapping_ad_id'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE,
  );
 $mapping = variable_get('os2intra_mapping');
 $form['os2intra_mapping']['os2intra_mapping_ad_id']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('AD id'),
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_ad_id']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    ),
  );
  $form['os2intra_mapping']['os2intra_mapping_ad_id']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_ad_id']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    ),
  );
  $form['os2intra_mapping']['os2intra_mapping_ad_id']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_ad_id']['input'],
  );
   $form['os2intra_mapping']['os2intra_mapping_ad_id']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['ad_id'])) ? $users['ad_id'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );

  $form['os2intra_mapping']['os2intra_mapping_ad_name'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE,
  );
  $form['os2intra_mapping']['os2intra_mapping_ad_name']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('AD name'),
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_ad_name']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    ),
  );
  $form['os2intra_mapping']['os2intra_mapping_ad_name']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_ad_name']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    ),
  );
  $form['os2intra_mapping']['os2intra_mapping_ad_name']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_ad_name']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_ad_name']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['ad_name'])) ? $users['ad_name'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );

  $form['os2intra_mapping']['os2intra_mapping_dn'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE,
  );
  $form['os2intra_mapping']['os2intra_mapping_dn']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('DN string'),
    '#required'      => TRUE,
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_dn']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_dn']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_dn']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )
  );
   $form['os2intra_mapping']['os2intra_mapping_dn']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_dn']['input'],
  );
   $form['os2intra_mapping']['os2intra_mapping_dn']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['dn'])) ? $users['dn'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );
  $form['os2intra_mapping']['os2intra_mapping_first_name'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_first_name']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Firstname'),
    '#required'      => TRUE,
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_first_name']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_first_name']['secondary'] = array(
    '#type'          => 'select',
    '#required'      => TRUE,
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_first_name']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_first_name']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_first_name']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_first_name']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['first_name'])) ? $users['first_name'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );
  $form['os2intra_mapping']['os2intra_mapping_last_name'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_last_name']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Lastname'),
    '#required'      => TRUE,
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_last_name']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_last_name']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_last_name']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_last_name']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_last_name']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_last_name']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['last_name'])) ? $users['last_name'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );
  $form['os2intra_mapping']['os2intra_mapping_department'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_department']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Department ID'),
    '#required'      => TRUE,
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_department']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_department']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_department']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_department']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_department']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_department']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['department'])) ? $users['department'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );
  $form['os2intra_mapping']['os2intra_mapping_department_shortname'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_department_shortname']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Department short name'),
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_department_shortname']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
    $form['os2intra_mapping']['os2intra_mapping_department_shortname']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_department_shortname']['secondary'],
      '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_department_shortname']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_department_shortname']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_department_shortname']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['department_shortname'])) ? $users['department_shortname'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );
  $form['os2intra_mapping']['os2intra_mapping_centre'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_centre']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Parent department ID'),
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_centre']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
   $form['os2intra_mapping']['os2intra_mapping_centre']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_centre']['secondary'],
     '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_centre']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_centre']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_centre']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['centre'])) ? $users['centre'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );
  $form['os2intra_mapping']['os2intra_mapping_centre_name'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_centre_name']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Parent department name'),
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_centre_name']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
   $form['os2intra_mapping']['os2intra_mapping_centre_name']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_centre_name']['secondary'],
     '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_centre_name']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_centre_name']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_centre_name']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['centre_name'])) ? $users['centre_name'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );
  $form['os2intra_mapping']['os2intra_mapping_employee_id'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_employee_id']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Employee id'),
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_employee_id']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_employee_id']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_employee_id']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_employee_id']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_employee_id']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_employee_id']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['employee_id'])) ? $users['employee_id'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );
  $form['os2intra_mapping']['os2intra_mapping_job_description'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_job_description']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Job description'),
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_job_description']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_job_description']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_job_description']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_job_description']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_job_description']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_job_description']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['job_description'])) ? $users['job_description'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );
  $form['os2intra_mapping']['os2intra_mapping_termination_date'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_termination_date']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Termination date'),
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_termination_date']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_termination_date']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_termination_date']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_termination_date']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_termination_date']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_termination_date']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['termination_date'])) ? $users['termination_date'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );
  $form['os2intra_mapping']['os2intra_mapping_start_date'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_start_date']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Start date'),
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_start_date']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_start_date']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_start_date']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_start_date']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_start_date']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_start_date']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['start_date'])) ? $users['start_date'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );
  $form['os2intra_mapping']['os2intra_mapping_phone'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_phone']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Phone Number'),
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_phone']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_phone']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_phone']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )

  );
  $form['os2intra_mapping']['os2intra_mapping_phone']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_phone']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_phone']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['phone'])) ? $users['phone'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );
  $form['os2intra_mapping']['os2intra_mapping_mobile'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_mobile']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Mobile Number'),
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_mobile']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_mobile']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_mobile']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_mobile']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_mobile']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_mobile']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['mobile'])) ? $users['mobile'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );
  $form['os2intra_mapping']['os2intra_mapping_email'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_email']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Email Address'),
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_email']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_email']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_email']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )

  );
  $form['os2intra_mapping']['os2intra_mapping_email']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_email']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_email']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['email'])) ? $users['email'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );
  $form['os2intra_mapping']['os2intra_mapping_birthdate'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_birthdate']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Birthdate'),
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_birthdate']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_birthdate']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_birthdate']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_birthdate']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_birthdate']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_birthdate']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['birthdate'])) ? $users['birthdate'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );
  $form['os2intra_mapping']['os2intra_mapping_department_name'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_department_name']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Department name'),
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_department_name']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_department_name']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_department_name']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_department_name']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_department_name']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_department_name']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['department_name'])) ? $users['department_name'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );


  $form['os2intra_mapping']['os2intra_mapping_opus_roles'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_opus_roles']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Opus roles'),
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_opus_roles']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_opus_roles']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_opus_roles']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_opus_roles']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_opus_roles']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_opus_roles']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['opus_roles'])) ? $users['opus_roles'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );

  $form['os2intra_mapping']['os2intra_mapping_leader'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_leader']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Leder/chef'),
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_leader']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_leader']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_leader']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_leader']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_leader']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_leader']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['leader'])) ? $users['leader'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );
  $form['os2intra_mapping']['os2intra_mapping_has_ad_value'] = array(
    '#type'          => 'fieldset',
    '#tree'          => TRUE
  );
  $form['os2intra_mapping']['os2intra_mapping_has_ad_value']['primary'] = array(
    '#type'          => 'select',
    '#title'         => t('Har AD'),
    '#options'       => $csv_cols,
    '#default_value' => $mapping['os2intra_mapping_has_ad_value']['primary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-primary-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_has_ad_value']['secondary'] = array(
    '#type'          => 'select',
    '#options'       => $csv_cols_fallback_1,
    '#default_value' => $mapping['os2intra_mapping_has_ad_value']['secondary'],
    '#attributes'    => array(
      'class'=> array('os2intra-mapping-fallback-field')
    )
  );
  $form['os2intra_mapping']['os2intra_mapping_has_ad_value']['input'] = array(
    '#type'          => 'textfield',
    '#default_value' => $mapping['os2intra_mapping_has_ad_value']['input'],
  );
  $form['os2intra_mapping']['os2intra_mapping_has_ad_value']['markup'] = array(
    '#type'   => 'item',
    '#markup' => (isset($users['has_ad'])) ? $users['has_ad'] : '',
    '#prefix' => '<div class="os2intra-mapping-markup-field">',
    '#suffix' => '</div>',
  );
  $form['cleanup_help_block'] = array(
    '#type'   => 'item',
    '#title'  => t('Cleanup script'),
    '#markup' => '<ul><li>Deletes all terms in the vocabulary "os2intra_organization_tax"</li><li>Deletes all users with only one role "authenticated user"</li></ul><br>Run using: Run using drush scr ../scripts/delete_imported_data.php',
  );
  return system_settings_form($form);
}

/**
 * Implements hook_form_validate()
 */
function os2intra_user_import_settings_validate(&$form, &$form_state) {
  $values = array();
  foreach ($form_state['values']['os2intra_mapping'] as $field => $value) {
      if (in_array($value['primary'], $values) && $value['primary'] != 'none') {
        drupal_set_message(t('Some of the columns are used twice in mapping, that is not an error if it has been done on purpose'), WATCHDOG_WARNING);
      }
      if ($value['secondary'] == 'input' && $value['input']=="") {
        form_set_error( 'os2intra_mapping]['. $field . '][input', t('Input fireld can\'t be empty'));
      }
      if ($value['secondary'] == $value['primary'] && ($value['primary'] != 'none' && $value['secondary'] != 'none') ) {
        form_set_error( 'os2intra_mapping]['. $field . '][secondary', t('Cannot use same column as primary and fallback value'));
      }
      $values[] = $value['primary'];
  }

  $centre_primary = $form_state['values']['os2intra_mapping']['os2intra_mapping_centre']['primary'];
  $centre_secondary = $form_state['values']['os2intra_mapping']['os2intra_mapping_centre']['secondary'];
  $department_primary = $form_state['values']['os2intra_mapping']['os2intra_mapping_department']['primary'];
  $department_secondary = $form_state['values']['os2intra_mapping']['os2intra_mapping_department']['secondary'];

  if (strcasecmp($centre_secondary, 'none') != 0) {
    if ($centre_secondary == $department_secondary || $centre_secondary == $department_primary) {
      form_set_error( 'os2intra_mapping][os2intra_mapping_centre][secondary', t('Cannot use same column department and parent department ID'));
    }
  }

  if (strcasecmp($department_secondary, 'none') != 0) {
    if ($centre_primary == $department_secondary) {
      form_set_error( 'os2intra_mapping][os2intra_mapping_department][secondary', t('Cannot use same column department and parent department ID'));
    }
  }
  if ($form_state['values']['os2intra_user_default_groups']) {
    $default_groups = explode(',', $form_state['values']['os2intra_user_default_groups']);
    foreach ($default_groups as $group) {
      $node = node_load($group);
      if (!$node || $node->type != $form_state['values']['os2intra_groups_node_type']) {
        form_set_error('os2intra_user_default_groups', t('Must be a valid NID of type organic groups (OG)'));
      }
    }
  }
}
