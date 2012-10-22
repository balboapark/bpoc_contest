<?php

function bpoc_contest_views_data() {

  $data = array();
  $data['bpoc_contest_votes']['table']['group'] = t('Contest');

  $data['bpoc_contest_votes']['vote_id'] = array(
    'title' => t('ID'),
    'help' => t('The ID'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
  );

  $data['bpoc_contest_votes']['nid'] = array(
    'title' => t('Node'),
    'help' => t('The particular node the vote is attached to'),
    'relationship' => array(
      'label' => t('Node'),
      'base' => 'node',
      'base field' => 'nid',
      'skip base' => array('node', 'node_revisions'),
    ),
  );
  $data['bpoc_contest_votes']['email_address'] = array(
    'title' => t('Email Address'), 
    'help' => t('The Email address of the user who cast the vote.'), 
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  return $data;
}

function bpoc_contest_views_data_alter(&$views_data) {
  $data = module_invoke_all('bpoc_contest_relationships');

  $data[] = array(
    'description' => t('nodes'),
    'content_type' => 'node',
    'base_table' => 'node',
    'content_id_column' => 'nid',
  );
}

